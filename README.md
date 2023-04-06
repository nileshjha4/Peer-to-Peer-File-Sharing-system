# SSD Assignment 3A - Python Programming
## Run the application
1. Install openssl: `sudo apt-get install openssl`
2. Compile tracker: `g++ tracker.cpp -o tracker -pthread`
3. Compile clients: `g++ client.cpp -o client -pthread -lcrypto`
### Assumptions
Application runs for only 1 tracker. Multiple tracker is not supported.
Filename is filepath (both relative & absolute is supported).
Correct commands will be passed, program may crash otherwise.w

### Client Commands
a. Run Client: 
`./client <IP>:<PORT> tracker_info.txt`
b. Create User Account: 
`create_user <user_id> <passwd>`
c. Login: 
`login <user_id> <passwd>`
d. Create Group: 
`create_group <group_id>`
e. Join Group: 
`join_group <group_id>`
f. Leave Group: 
`leave_group <group_id>`
g. List pending join: 
`requests list_requests <group_id>`
h. Accept Group Joining Request: 
`accept_request <group_id> <user_id>`
i. List All Group In Network: 
`list_groups`
j. List All sharable Files In Group: 
`list_files <group_id>`
k. Upload File: 
`upload_file <file_path> <group_id >`
l. Download File: 
`download_file <group_id> <filepath> <destination_path>`
m. Logout: 
`logout`
n. Show_downloads: 
`show_downloads`
o. Stop sharing: 
`stop_share <filepath> <groupid>`