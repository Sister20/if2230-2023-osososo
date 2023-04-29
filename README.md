# OSOSOSO OS

This project is made to fulfill the 1st, 2nd, and 3rd milestone for the Operating System. It is a simple os that can do simple command like ls, mkdir, cp, cd, cat, and mv. It also implemented by using multiprogramming and message passing

## Made by

| Name                           |   Nim    |
| ------------------------------ | :------: |
| Muhammad Hanan                 | 13521041 |
| Vieri Fajar Firdaus            | 13521099 |
| Muhammad Rizky Sya’ban         | 13521119 |
| Saddam Annais Shaquille        | 13521121 |


## Screenshots

![image](https://user-images.githubusercontent.com/73151449/166110062-b8515469-083e-41d9-be93-c1f1424de759.png)
![image](https://user-images.githubusercontent.com/73151449/166110079-6800f7ec-8e67-4481-9ed5-25d2f5db3d8c.png)

## Technologies Used

1. [VirtualBox](https://www.virtualbox.org/)
2. [Window Subsytem for Linux](https://docs.microsoft.com/en-us/windows/wsl/install)
3. [Ubuntu 20.04 LTS](https://releases.ubuntu.com/20.04/)
4. [Nasm](https://www.nasm.us/)
5. [bcc](https://bochs.sourceforge.io/)
6. bochs
7. c

## Features

This OS was in implementation from the boilerplate given by sister20. In this OS you can use simple command like `cat` to read file, `ls` to list file and many more. To see more command check the command section. This OS is made using bcc, bochs, c, and asm. 

## Program Features
1. kernel
2. shell
4. cd
5. cp
6. cat
7. ls
8. mv
9. mkdir

## Setup

1. Install all the requirements in the technologies section

```
sudo apt update
sudo apt install gcc nasm make qemu-system-x86 genisoimage
```

2. Run the virtual machine using VirtualBox (Another option by using [WSL2](https://github.com/Sister19/WSL-Troubleshoot))
3. clone the github repository by using `git clone https://github.com/Sister20/if2230-2023-osososo`
4. run `make all run` and eureka

## Command
1. `cd <target_dir>` - move from current directory to target dir
2. `ls [folder]` - list all the files or folder in folder name
3. `mkdir <folder_name>` - to create new folder in current dir
4. `cat <file_name>` - to read and display file
6. `cp <file> <target>` - copy file to target dir if no folder exists will rename to target
7. `rm` <file> - remove file from current directory
8. `mv <file > <target_dir>/[new_name]` - move file or folder in current dir to the destination and be able to rename the file if the argument is specified

## Usage

1. Run the kernel program and a welcome page will be displayed. Press any key to clear the screen and use the program.
2. After you run the OS the clearFunction will run by default and it will clear the screen leaving the shell with the location of curdir
3. You can type any character, or command in that will go to the buffer with 128 byte size.
4. If you pressed enter, the text you typed before will be process as command.
