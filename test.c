

int8_t write(struct FAT32DriverRequest request) {
    struct FAT32DirectoryTable *dir_table = (struct FAT32DirectoryTable *) driver_state.dir_table_buf.table;
    struct FAT32DirectoryEntry *dir_entry = NULL;

    // Search for existing directory entry
    int8_t entry_found = 0;
    uint32_t parent_cluster_number = request.parent_cluster_number;
    uint32_t current_cluster_number = parent_cluster_number;
    while (1) {
        // Read current cluster
        read_clusters(&driver_state.cluster_buf, current_cluster_number, 1);

        // Traverse directory table
        for (int i = 0; i < CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry); i++) {
            dir_entry = &(dir_table->table[i]);

            // Check if directory entry is empty or deleted
            if (dir_entry->name[0] == '\0' || dir_entry->name[0] == 0xE5) {
                // Empty directory entry found
                entry_found = 0;
                break;
            }

            // Check if directory entry matches the requested name and extension
            if (strncmp(dir_entry->name, request.name, 8) == 0 && strncmp(dir_entry->ext, request.ext, 3) == 0) {
                // Directory entry found
                entry_found = 1;
                break;
            }
        }

        if (entry_found) {
            break;
        }

        // Check if current cluster is the last cluster in the chain
        uint32_t next_cluster_number = driver_state.fat_table.cluster_map[current_cluster_number];
        if (next_cluster_number >= FAT32_FAT_END_OF_FILE) {
            // End of chain, create new cluster
            next_cluster_number = find_free_cluster();
            if (next_cluster_number == 0) {
                // No free cluster available
                return -1;
            }

            // Link current cluster to new cluster
            driver_state.fat_table.cluster_map[current_cluster_number] = next_cluster_number;
            driver_state.fat_table.cluster_map[next_cluster_number] = FAT32_FAT_END_OF_FILE;

            // Write updated FAT to disk
            write_clusters(&driver_state.fat_table, CLUSTER_MAP_SIZE, 1);
        }

        // Continue to next cluster in the chain
        current_cluster_number = next_cluster_number;
    }

    // Check if file/folder already exists
    if (entry_found) {
        return 1;
    }

    // Create new directory entry
    dir_entry->attribute = request.buffer_size == 0 ? ATTR_SUBDIRECTORY : 0;
    strncpy(dir_entry->name, request.name, 8);
    strncpy(dir_entry->ext, request.ext, 3);
}




//================================================================================


// Error code: 0 success - 1 file/folder already exist - 2 invalid parent cluster - -1 unknown

int8_t write(struct FAT32DriverRequest request) {
    // Check if parent cluster exists
    if (request.parent_cluster_number < ROOT_CLUSTER_NUMBER || request.parent_cluster_number >= driver_state.fat_table.cluster_map_size) {
        return 2; // Invalid parent cluster
    }

    // Read parent directory table
    if (read_directory(request) != 0) {
        return -1; // Unknown error
    }

    // Check if file/folder already exists
    for (int i = 0; i < CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry); i++) {
        struct FAT32DirectoryEntry *dir_entry = &(driver_state.dir_table_buf.table[i]);
        if (dir_entry->user_attribute == UATTR_NOT_EMPTY && strcmp(dir_entry->name, request.name) == 0 && strcmp(dir_entry->ext, request.ext) == 0) {
            return 1; // File/folder already exists
        }
    }

    // Find the first empty directory entry
    struct FAT32DirectoryEntry *empty_entry = NULL;
    for (int i = 0; i < CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry); i++) {
        struct FAT32DirectoryEntry *dir_entry = &(driver_state.dir_table_buf.table[i]);
        if (dir_entry->user_attribute != UATTR_NOT_EMPTY) {
            empty_entry = dir_entry;
            break;
        }
    }

    // If buffer_size == 0, create a new directory
    if (request.buffer_size == 0) {
        // Check if there is an empty directory entry
        if (empty_entry == NULL) {
            return -1; // Unknown error
        }

        // Initialize directory table
        init_directory_table(&driver_state.dir_table_buf, request.name, request.parent_cluster_number);

        // Write directory table
        uint32_t dir_cluster = empty_entry->cluster_low | (empty_entry->cluster_high << 16);
        write_clusters(&driver_state.dir_table_buf, dir_cluster, 1);

        // Update parent directory entry
        empty_entry->user_attribute = UATTR_NOT_EMPTY;
        empty_entry->attribute = ATTR_SUBDIRECTORY;
        empty_entry->cluster_low = dir_cluster & 0xFFFF;
        empty_entry->cluster_high = dir_cluster >> 16;
        empty_entry->filesize = 0;
    } else {
        // Check if there is an empty directory entry
        if (empty_entry == NULL) {
            return -1; // Unknown error
        }

        // Write file data
        uint32_t file_cluster = empty_entry->cluster_low | (empty_entry->cluster_high << 16);
        write_clusters(request.buf, file_cluster, (request.buffer_size + CLUSTER_SIZE - 1) / CLUSTER_SIZE);

        // Update directory entry
        empty_entry->user_attribute = UATTR_NOT_EMPTY;
        empty_entry->attribute = 0;
        empty_entry->cluster_low = file_cluster & 0xFFFF;
        empty_entry->cluster_high = file_cluster >> 16;
        empty_entry->filesize = request.buffer_size;
    }

    // Write parent directory table
    uint32_t parent_dir_cluster = cluster_to_lba(request.parent_cluster_number);
    write_clusters(&driver_state.dir_table_buf, parent_dir_cluster, 1);

    return 0; // Success
}


int8_t read(struct FAT32DriverRequest request) {
    // Check if file exists and is not a directory
    struct FAT32DirectoryEntry entry;
    int8_t err = read_directory_entry(&entry, request.name, request.ext, request.parent_cluster_number);
    if (err != 0 || (entry.attribute & ATTR_SUBDIRECTORY) != 0) {
        return 1; // Not a file
    }

    // Check if buffer is large enough to hold file
    if (request.buffer_size < entry.filesize) {
        return 2; // Not enough buffer
    }

    // Read file data into buffer
    uint32_t cluster_number = entry.cluster_low | (entry.cluster_high << 16);
    uint32_t remaining_bytes = entry.filesize;
    uint8_t *buffer = (uint8_t *)request.buf;
    while (remaining_bytes > 0) {
        uint32_t bytes_to_read = min(remaining_bytes, CLUSTER_SIZE);
        read_clusters(driver_state.cluster_buf.buf, cluster_number, 1);
        memcpy(buffer, driver_state.cluster_buf.buf, bytes_to_read);

        remaining_bytes -= bytes_to_read;
        buffer += bytes_to_read;
        cluster_number = get_fat_entry(cluster_number);
        if (cluster_number >= FAT32_FAT_END_OF_FILE) {
            break;
        }
    }

    return (remaining_bytes > 0) ? 3 : 0; // Not found or success
}