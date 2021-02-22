#include <stdio.h>
#include <string.h> 
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h> // for inet_addr
#include <unistd.h>  
#include <pthread.h>
#include "json.h" //json.h and json.c are the libraries that i use to parse JSON object. The makefile and these two documents are also uploaded to Sakai and Github. To execute the program I need them to be in the same folder.
#include "json.c" //The library is founded from Github.

#define MAX_GROUP 15        //MAX NUMBER OF GROUPS IS 15.
#define GROUP_CAPACITY 15   //CAPACITY OF EACH GROUP IS 15.
#define PORT_NO 3205        //PORT NO IS 3205.

typedef struct UserStr //USER STRUCT
{
    char phoneNumber[10];
    char groupName[100];
    int groupNumber;
    int socketNo;
    int index;
} USER;

typedef struct GroupStr //GROUP STRUCT
{
    char name[100];
    char password[100];
    USER Users[GROUP_CAPACITY];
    USER owner;
    int userCount;
} GROUP;


GROUP Groups[MAX_GROUP];
int groupCount = 0;


void groupReset(int i) //This function resets the given group and its properties.
{
    Groups[i].userCount = 0;
    strcpy(Groups[i].name, "");
    strcpy(Groups[i].password, "");
    for (int j = 0; j < MAX_GROUP; j++)
    {
        strcpy(Groups[i].Users[j].phoneNumber, "");
        strcpy(Groups[i].Users[j].groupName, "");
        Groups[i].Users[j].socketNo = -1;
        Groups[i].Users[j].index = -1;
    }
}


int createGroup(USER *user, char *GroupName) //This function creates the group.
{
    if (groupCount < MAX_GROUP)
    {   
        for (int i = 0; i < MAX_GROUP; i++)
        {
            if (Groups[i].userCount == 0) //An empty index from Groups array is found. All the properties of groups are filled.
            {
                Groups[i].userCount++;
                strcpy(user->groupName, GroupName);
                strcpy(Groups[i].name, GroupName);
                strcpy(Groups[i].owner.phoneNumber, user->phoneNumber); 
                user->groupNumber = i;
                user->index = 0;
                Groups[i].Users[0] = *user;

                char message[100];
                strcpy(message, "Please enter group password:");
                send(user->socketNo, message, strlen(message), 0);
                recv(user->socketNo, Groups[i].password, 100, 0);

                char *msg = "Group is created.\n";
                send(user->socketNo, msg, strlen(msg), 0);
                groupCount++;
                
                char gname[100]; //That block sends the groupname to clientside to use it later. 
                sprintf(gname, "-gname***%s", GroupName);
                send(user->socketNo, gname, 100, 0);
                break;
            }
        } 
    }
}


void sendMessageToGroup(USER *user, char* from, char* to, char *message)
{   
    if((strcmp(from, user->phoneNumber) == 0) && (strcmp(to, user->groupName) == 0)){ //Controls if the given properties fit to the ones that is stored. If the group names and phone numbers are same, it sends.
        char msg[200];
        char msg2[200];
        strcpy(msg, user->phoneNumber);
        strcat(msg, ": ");
        strcat(msg, message);
        strcpy(msg2, "You: ");
        strcat(msg2, message);

        for (int i = 0; i < MAX_GROUP; i++)
        {    
            if (Groups[user->groupNumber].Users[i].socketNo != user->socketNo && Groups[user->groupNumber].Users[i].socketNo != -1) //For the other users in the group.
            {
                send(Groups[user->groupNumber].Users[i].socketNo, msg, strlen(msg), 0);
            }
            else if (Groups[user->groupNumber].Users[i].socketNo == user->socketNo) //For the sender to make them see their message.
            {
                send(Groups[user->groupNumber].Users[i].socketNo, msg2, strlen(msg2), 0);
            }
        }
    }
     
}

void joinGroup(USER *user, char *group)
{
    int isGroupExist = -1;
    int entrance = -1;
    char groupn[100];
    for (int i = 0; i < MAX_GROUP; i++)
    {
        if ((strcmp(Groups[i].name, group) == 0) || (strcmp(Groups[i].owner.phoneNumber, group) == 0)) //Users can join to the group with owner's phone number or the group name.
        {
            isGroupExist = 1;
            if (Groups[i].userCount < GROUP_CAPACITY)
            {
                char message[100];
                char temp[100];
                strcpy(message, "This group is private. Please enter the password:"); //Each group is private. Password is needed.
                send(user->socketNo, message, strlen(message), 0);
                recv(user->socketNo, temp, 100, 0);
                if (strcmp(Groups[i].password, temp) == 0)
                {
                    entrance = 1;
                }
                else
                {
                    strcpy(message, "Password is incorrect.");
                    send(user->socketNo, message, strlen(message), 0);
                }
                if (entrance == 1)
                {
                    for (int j = 0; j < GROUP_CAPACITY; j++)
                    {
                        if (Groups[i].Users[j].socketNo == -1)
                        {
                            user->groupNumber = i;
                            user->index = j;
                            strcpy(user->groupName, Groups[i].name);
                            Groups[i].userCount++;
                            Groups[i].Users[j] = *user;
                            char message[100];
                            strcpy(message, "You the entered the group.");
                            send(user->socketNo, message, strlen(message), 0);
                            
                            if(strcmp(Groups[i].name, group) == 0){ //Groupname is sent to the client to use it later. 
                                sprintf(groupn, "-gname***%s", group);
                                send(user->socketNo, groupn, 100, 0);
                            }
                            else if(strcmp(Groups[i].owner.phoneNumber, group) == 0){
                                sprintf(groupn, "-gname***%s", Groups[i].name);
                                send(user->socketNo, groupn, 100, 0);
                            }
                            memset(groupn,"",sizeof(groupn)); //To clear the array.
                            break;
                        }
                    }
                }
            }
            else //if the group is full
            {
                char *msg = "This group is full.\n";
                send(user->socketNo, msg, strlen(msg), 0);
                break;
            }
        }
    }
    if(isGroupExist == -1) //There is no group with this name.
    {
        char *msg = "No such a group with that name.\n";
        send(user->socketNo, msg, strlen(msg), 0);
    }
}

void *connection(void *socket_desc)
{
    int socket = *((int *)socket_desc);
    char usr[100];
    char username[100];
    USER user;
    user.socketNo = socket;
    user.groupNumber = -1;
    char message[200] = "Please enter your phone number :"; //Phone number of the user is taken.
    send(socket, message, strlen(message), 0);
    recv(socket, user.phoneNumber, 100, 0);
    strcpy(usr, user.phoneNumber);
    sprintf(username, "-username %s", usr); //Phone number is sent to the client to use it later.
    send(socket, username, 100, 0);

    while (socket != -1) //This iteration goes on until the socket is -1.
    {
        char msg[100] = "";
        char temp[100] = "";
        char *temp1;
        recv(socket, msg, 100, 0);
        puts(msg);
        strcpy(temp, msg);
        char *token = (char *)malloc(100 * sizeof(char));
        char *phoneNumber = (char *)malloc(100 * sizeof(char));
        char *groupName = (char *)malloc(100 * sizeof(char));

        if (strcmp(msg, "-exit") == 0)
        {
            socket = -1;
            strcpy(message, "You exit the program.\n");
            free(socket_desc);
            send(user.socketNo, message, strlen(message), 0); 
        }
        token = strtok(temp, " ");
        if ((strcmp(token, "-gcreate") == 0 && strlen(msg) > 8))
        {
            if (user.groupNumber == -1) //The user has no groups yet
            {
                temp1 = strstr(msg, " ");
                temp1 = temp1 + 1;
                strcpy(groupName, temp1);
                phoneNumber = strtok(temp1, "+");
                if (strcmp(phoneNumber, user.phoneNumber) == 0)
                {
                    groupName = strtok(NULL, "+");
                    createGroup(&user, groupName);
                }
                else
                {
                    strcpy(message, "Please enter your phone number correctly!\n");
                    send(socket, message, strlen(message), 0);
                }
            }
            else
            {
                strcpy(message, "You have already joined to another group... You cannot create another group.\n");
                send(socket, message, strlen(message), 0);
            }
        }
        else if ((strcmp(token, "-gcreate") == 0))
        {
            strcpy(message, "Please enter a group name.\n");
            send(socket, message, strlen(message), 0);
        }
        else if (strcmp(token, "-join") == 0)
        {
            if (strlen(msg) > 5)
            {
                if (user.groupNumber == -1) //This means the user is not in any group.
                {
                    temp1 = strstr(msg, " ");
                    temp1 = temp1 + 1;
                    joinGroup(&user, temp1);
                }
                else
                {
                    strcpy(message, "You have already joined the group.\n");
                    send(socket, message, strlen(message), 0);
                }
            }
        }
        else if (strcmp(token, "-send") == 0)
        {
            if (user.groupNumber > -1)
            {
                if (strlen(msg) > 5) //There is a message
                {
                    temp1 = strstr(msg, " ");
                    temp1 = temp1 + 1;
                    char string[100];
                    JSONObject *jsonMessage = parseJSON(temp1); //This is a library function. 
                    sendMessageToGroup(&user, jsonMessage ->pairs[0].value ->stringValue, jsonMessage ->pairs[1].value ->stringValue, jsonMessage ->pairs[2].value ->stringValue); //from, to, and messages are parsed from the JSON.
                }
                else
                {
                    strcpy(message, "Please enter your message:\n");
                    send(socket, message, strlen(message), 0);
                }
            }
        }

        else if (strcmp(token, "-exit") == 0) //This is the command -exit groupName, not the exit from the program.
        {
            if (strlen(msg) > 5) 
            {
                token = strstr(msg, " ");
                token = token + 1;
                if (user.groupNumber == -1) //User doesnt have any group, so they cannot leave.
                {
                    strcpy(message, "You haven't joined any group.\n");
                    send(socket, message, strlen(message), 0);
                }
                else if (strstr(user.groupName, token) != NULL)
                {
                    Groups[user.groupNumber].userCount--; //This user is leaving, count is decreased.
                    if (Groups[user.groupNumber].userCount != 0) 
                    {
                        USER tempUser;
                        Groups[user.groupNumber].Users[user.index] = tempUser;
                        Groups[user.groupNumber].Users[user.index].socketNo = -1;
                    }
                    else //If nobody left in the group, group is deleted.
                    {
                        groupReset(user.groupNumber);
                        groupCount--; 
                    }
                    user.groupNumber = -1;
                    user.index = -1;
                    strcpy(user.groupName, "");
                    strcpy(message, "You quit the group\n");
                    send(socket, message, strlen(message), 0);
                    char temp4[100];
                    sprintf(temp4, "-gname***no_group");
                    send(socket, temp4, 100, 0);
                }
                else
                {
                    strcpy(message, "Wrong group name...\n");
                    send(socket, message, strlen(message), 0);
                }
            }
            else
            {
                strcpy(message, "Please enter group name....\n");
                send(socket, message, strlen(message), 0);
            }
        }
    }
    return 0;
}

int main(int argc, char *argv[])
{
    for (int i = 0; i < MAX_GROUP; i++)
    {
        groupReset(i);
    }
    int socket_desc, new_socket, c, *new_sock;
    struct sockaddr_in server, client;
    char *message;

    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1)
    {
        puts("Could not create socket");
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT_NO);

    if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        puts("Binding failed");
        return 1;
    }
    listen(socket_desc, 100);
    c = sizeof(struct sockaddr_in);
    while ((new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t *)&c)))
    {
        int socket_des = new_socket;
        puts("Connection accepted");
        message = "Welcome to DEU Signal!\n";
        write(new_socket, message, strlen(message));
        pthread_t sniffer;
        new_sock = malloc(1);
        *new_sock = new_socket;
        if (pthread_create(&sniffer, NULL, connection,
                           (void *)new_sock) < 0)
        {
            puts("Could not create thread");
            return 1;
        }
    }
    return 0;
}
