# Server Setup

For the server, we used DigitalOcean.

1. Create the new project.
1. Under the project, create a new droplet -- this is the server.
1. In the droplet, go to the Access menu, and Launch Console. (Or launch console through the upper right hand corner, where it says "Console")
1. It will prompt for the username and password. The username is root, and the password is emailed to you when you create the droplet. If you did not receive an email, try resetting the root password.
1. After changing the default password to your own custom password, login.
1. To add users, run `adduser <username>`, and then fill out the prompted requested information. Now they should be able to access the server.

# Connecting to the Server

1. To connect to the server, run the command `ssh <username>@<ip address>`. The IP address should be on the top left corner on the droplet (labeled ipv4).
1. Enter the password.
1. Depending on the server permissions, you may need to set up SSH keys to access it.

# Setting up SSH keys

1. To generate a key, run `ssh-keygen`.
1. Run `ssh-copy-id -i ~/.ssh/id_rsa.pub <username>@<ip address>`
1. Now the SSH key should be set up. Try running `ssh <username>@<ip address>`. If it doesn't prompt you for a password, then the SSH key was successfully set.

# User Management (Adding/Deleting Users)

1. To add new users, run `adduser --home /home/<new user> <new user>`. This will set the new user to only have write permissions to their home directory (/home/\<new user\>).
1. To set it so that only that user has read/write access to their home directory, run `chmod 0750 /home/<new user>`.
   - To make this the default behavior, open /etc/adduser.conf, and change the line `DIR_MODE=0755` to `DIR_MODE=0750`
   - NOTE: For the CoCode server, we have changed the permissions from 0750 to 0550, so the user only has read access to their home directory.
1. To delete a user and its home directory, run `deluser --remove-home <user>`.
   - NOTE: If you run into issues deleting the user (such as a user is currently logged in when you attempt to delete the user), try running the following code: `sudo killall -u <username> && sudo deluser --remove-home -f <username>`.
1. To change a password, run `passwd`. To change a specific user's password, run `passwd <user>`.

# User Management (continued)

1. For CoCode, we have a guest account set to have the [guest_add_user.sh](https://github.com/jennifer4569/CoCode/blob/master/source_control/server_files/guest_add_user.sh) file, which signals to the server to register for an account. 
1. To ensure that this file cannot be modified by anyone (including the root), we ran the following command: `$ sudo chattr +i guest_add_user.sh`. This sets the file to be immutable. 
   - NOTE: To undo this command, run `$ sudo chattr -i guest_add_user.sh`.
   
# Setting Permissions for Specific Commands

1. To disable a command, first find the absolute path of command: `which <command>`.
1. Run: `setfacl -m u:<username>:--- <path/to/command>`.
1. To see the command's permission settings, run: `getfacl <path/to/command>`.

For example:
```
$ which chmod
/bin/chmod
$ setfacl -m u:mike:--- /bin/chmod
```

# Useful Commands

Uploading a file to the server (NOTE: This command only works in Mac/Linux, as well as WSL):
`rsync -avz path/to/upload/file <username>@<ipaddress>:path/to/destination`

Switching users (from \<user1\> to \<user2\>) `su <user2>`

Killing a process occupying a certain port: `sudo fuser -k <port_number>/tcp`

# Allowing Clients Connect to Server

First, set up an ssh key (See above).
Then, in the server, run `sudo ufw allow ssh`
This will allow the client to connect to the server using the designated port.



