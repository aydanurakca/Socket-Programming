# Socket-Programming

This is a instant messaging application programmed in C.

Server executes commands which are given by clients, controls and manages the system. When the client connects successfully to the server, they enter their private phone number initially. Then they become defined in the server system.

The commands are described in below can be used in the system:
● -gcreate phone_number + group_name: Creates a new specified group. The system will ask to define a password to the admin of the group. All groups are protected with non-encrypted passwords.
● -join username/group_name: This command is to join to a specified group. Clients can enter the group name or the admin's username to join the group. They must know the password of the group for entering.
● -exit group_name: With this command clients can quit from the group that they are in. The given group name must be the same with the current group name.
● -send message_body: Clients can send a JSON-formatted message to the group that they are in.
● -whoami: Shows the client's phone number information.
● -exit: This command is used to exit the program.

json.h and json.c are the libraries that are used to parse JSON object.

Compilation of the files are in that format:
-gcc server.c -o server.out -lpthread
-gcc client.c -o client.out -lpthread
