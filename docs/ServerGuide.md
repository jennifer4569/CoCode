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

1. To add new users, run `adduser --home /home/<new user> <new user>`. This will set the new user to only have write permissions to their home directory (/home/<new user>).
1. To set it so that only that user has access to their home directory, run `chmod 0750 /home/<new user>`.
   - To make this the default behavior, open /etc/adduser.conf, and change the line `DIR_MODE=0755` to `DIR_MODE=0750`
1. To delete a user and its home directory, run `deluser --remove-home <user>`.

# Useful Commands

Uploading a file to the server (NOTE: This command only works in Mac/Linux, as well as WSL):
`rsync -avz path/to/upload/file <username>@<ipaddress>:path/to/destination`

Switching users (from <user1> to <user2>) `su <user2>`