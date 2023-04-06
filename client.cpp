#include <bits/stdc++.h>
#include <cstring>
#include <fstream>
#include <netinet/in.h>
#include <netdb.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <thread>
#include <unistd.h>
#include <arpa/inet.h>
#include <openssl/sha.h>

using namespace std;

bool loggedIn = false;
int portno;
int trackerPort;
pair<string,string> credentials;
string trackerHost;

void accept_request(string command, vector<string> tokens);
int cmpHash(unsigned char hash[20], unsigned char hash2[20]);
int connect_tracker();
int connect_peer(int port);
void create_user(string command, vector<string> tokens);
void create_group(string command, vector<string> tokens);
void download_file(string command, vector<string> tokens);
void download_chunk(int chunk_no, int portno, string filepath, unordered_map<int, vector<char>> &file);
void error(const char *msg);
void join_group(string command, vector<string> tokens);
void leave_group(string command, vector<string> tokens);
void list_requests(string command, vector<string> tokens);
void list_groups(string command, vector<string> tokens);
void list_files(string command, vector<string> tokens);
void login(string command, vector<string> tokens);
void logout();
void server_handle(int clientfd);
void select_chunks(string command, vector<string>tokens, vector<string> ports);
void socser(int portno);
void show_downloads(string command, vector<string> tokens);
void stop_share(string command, vector<string> tokens);
void tokenizeStr(string str, vector<string> &tokens);
void upload_file(string command, vector<string> tokens);
void calc_hash(vector<char> buffer, unsigned char (&hash)[20]);



void accept_request(string command, vector<string> tokens){
    int tr_fd = connect_tracker();
    int n;
    if(tr_fd < 0){
        cout<<"...Error: connecting to tracker\n";
        return;
    }
    command = command + " " + credentials.first;
    n = send(tr_fd,command.c_str(),command.length(), 0);
    if (n < 0){
        cout<<"...Error: sending request to  tracker!\n";
        close(tr_fd);
        return;
    }
    char buffer[257];
    bzero(buffer,257);
    n = read(tr_fd,buffer,257);  
    if (n < 0){
        cout<<"...Tracker ack for {request accept} request failed!\n";
        close(tr_fd);
        return;
    }
    if(buffer[0]=='1'){
        cout<<"...Request accepted succesfully.\n";
    }else if(buffer[0]=='2'){
        cout<<"...You're not admin.\n";
    }else if(buffer[0]=='3'){
        cout<<"...Group doesn't exists.\n";
    }else if(buffer[0] == '4'){
        cout<<"4\n";
    }else{
        cout<<"...Request accept failed.\n";
    }
    close(tr_fd);
    return;    
}

void calc_hash(vector<char> buffer, unsigned char (&hash)[20]){
    vector<unsigned char> buffer2;
    for(auto i: buffer) buffer2.push_back(i);
    SHA1(&buffer2[0], buffer.size(), hash);
}

int cmpHash(unsigned char hash[20], unsigned char hash2[20]){
    for(int i=0; i<20; i++){
        if(hash[i] != hash2[i]){
            return -1;
        }
    }
    return 0;
}

int connect_peer(int port){
    int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0){
        cout<<"...ERROR opening socket.\n";
        return -1;
    }
    server = gethostbyname(trackerHost.c_str());
    if (server == NULL) {
        fprintf(stderr,"Host of tracker is Invalid!\n");
        return -1;
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(port);
    int n=connect(sockfd,(struct sockaddr *) &serv_addr, sizeof(serv_addr)); 
    if (n< 0){
        return -1;
    } 
    return sockfd;
}

int connect_tracker(){
    int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0){
        cout<<"...ERROR opening socket.\n";
        return -1;
    }
    server = gethostbyname(trackerHost.c_str());
    if (server == NULL) {
        fprintf(stderr,"Host of tracker is Invalid!\n");
        return -1;
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(trackerPort);
    if (connect(sockfd,(struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
        return -1;
    } 
    return sockfd;
}

void create_user(string command, vector<string> tokens){
    int tr_fd = connect_tracker();
    int n;
    if(tr_fd < 0){
        cout<<"...Error: connecting to tracker\n";
        return;
    }
    command = command + " " + to_string(portno);
    n = send(tr_fd,command.c_str(),command.length(), 0);
    if (n < 0){
        cout<<"...Error: sending credentials to  tracker!\n";
        close(tr_fd);
        return;
    }
    char buffer[257];
    bzero(buffer,257);
    n = read(tr_fd,buffer,257);  
    if (n < 0){
        cout<<"...Tracker ack for {create_user} request failed!\n";
        close(tr_fd);
        return;
    }
    if(buffer[0]=='0'){
        cout<<"...User already exits. Please try logging in.\n";
    }else if(buffer[0]=='1'){
        credentials = make_pair(tokens[1], tokens[2]);
        cout<<"...User registered succesfully.\n";
        loggedIn = true;
    }else{
        cout<<"...Invalid Acknowledgement.\n";
    }
    close(tr_fd);
    return;    
}

void create_group(string command, vector<string> tokens){
    int tr_fd = connect_tracker();
    int n;
    if(tr_fd < 0){
        cout<<"...Error: connecting to tracker\n";
        return;
    }
    command = command + " " + credentials.first;
    n = send(tr_fd,command.c_str(),command.length(), 0);
    if (n < 0){
        cout<<"...Error: sending request to  tracker!\n";
        close(tr_fd);
        return;
    }
    char buffer[257];
    bzero(buffer,257);
    n = read(tr_fd,buffer,257);  
    if (n < 0){
        cout<<"...Tracker ack for {create_group} request failed!\n";
        close(tr_fd);
        return;
    }
    if(buffer[0]=='1'){
        cout<<"...Group created succesfully.\n";
    }else if(buffer[0]=='2'){
        cout<<"...Group already exists.\n";
    }else{
        cout<<"...Group creation failed.\n";
    }
    close(tr_fd);
    return;    
}

void download_file(string command, vector<string> tokens){
    int tr_fd = connect_tracker();
    int n;
    if(tr_fd < 0){
        cout<<"...Error: connecting to tracker\n";
        return;
    }
    //send cmd
    command = command + " " + credentials.first;
    n = send(tr_fd,command.c_str(),command.length(), 0);
    if (n < 0){
        cout<<"...Error: sending request to  tracker!\n";
        close(tr_fd);
        return;
    }
    char buffer[4096];
    bzero(buffer,4096);
    n = read(tr_fd,buffer,4095);  
    if (n < 0){
        cout<<"...Tracker ack for {list_files} request failed!\n";
        close(tr_fd);
        return;
    }
    vector<string> ports;
    tokenizeStr(string(buffer), ports);
    if(buffer[0]=='1'){
        if(ports.size()>2){
            select_chunks(command, tokens, ports);
            cout<<"...Files downloaded succesfully.\n";
        }else{
            cout<<"...No seeders found for the file.\n";
        }
    }else if(buffer[0]=='2'){
        cout<<"...You're not part of the group.\n";
    }else if(buffer[0]=='3'){
        cout<<"...Group doesn't exists\n";
    }else if(buffer[0]=='4'){
        cout<<"...File is not available in the group.\n";
    }else if(buffer[0]=='5'){
        cout<<"...You're already seeding/downloaded the file.\n";   
    }else{
        cout<<"...File downloading failed.\n";
    }
    close(tr_fd);
    return;    
}

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

void join_group(string command, vector<string> tokens){
    int tr_fd = connect_tracker();
    int n;
    if(tr_fd < 0){
        cout<<"...Error: connecting to tracker\n";
        return;
    }
    command = command + " " + credentials.first;
    n = send(tr_fd,command.c_str(),command.length(), 0);
    if (n < 0){
        cout<<"...Error: sending request to  tracker!\n";
        close(tr_fd);
        return;
    }
    char buffer[257];
    bzero(buffer,257);
    n = read(tr_fd,buffer,257);  
    if (n < 0){
        cout<<"...Tracker ack for {join_group} request failed!\n";
        close(tr_fd);
        return;
    }
    if(buffer[0]=='1'){
        cout<<"...Group join request sent succesfully.\n";
    }else if(buffer[0]=='2'){
        cout<<"...You're already a member of the group.\n";
    }else if(buffer[0]=='3'){
        cout<<"...Group doesn't exists\n";
    }else{
        cout<<"...Group joining failed.\n";
    }
    close(tr_fd);
    return;    
}

void list_files(string command, vector<string> tokens){
    int tr_fd = connect_tracker();
    int n;
    if(tr_fd < 0){
        cout<<"...Error: connecting to tracker\n";
        return;
    }
    //send cmd
    command = command + " " + credentials.first;
    n = send(tr_fd,command.c_str(),command.length(), 0);
    if (n < 0){
        cout<<"...Error: sending request to  tracker!\n";
        close(tr_fd);
        return;
    }
    char buffer[4096];
    bzero(buffer,4096);
    n = read(tr_fd,buffer,4095);  
    if (n < 0){
        cout<<"...Tracker ack for {list_files} request failed!\n";
        close(tr_fd);
        return;
    }
    if(buffer[0]=='1'){
        cout<<"...Listing files\n";
        cout<<string(buffer)<<endl;
    }else if(buffer[0]=='2'){
        cout<<"...You're not part of the group.\n";
    }else if(buffer[0]=='3'){
        cout<<"...Group doesn't exists\n";
    }else{
        cout<<"...Files listing failed.\n";
    }
    close(tr_fd);
    return;    
}

void list_groups(string command, vector<string> tokens){
    int tr_fd = connect_tracker();
    int n;
    if(tr_fd < 0){
        cout<<"...Error: connecting to tracker\n";
        return;
    }
    n = send(tr_fd,command.c_str(),command.length(), 0);
    if (n < 0){
        cout<<"...Error: sending request to  tracker!\n";
        close(tr_fd);
        return;
    }
    char buffer[4097];
    bzero(buffer,4097);
    n = read(tr_fd,buffer,4097);  
    if (n < 0){
        cout<<"...Tracker ack for {list_group} request failed!\n";
        close(tr_fd);
        return;
    }
    if(buffer[0]=='1'){
        string requests(buffer);
        vector<string> tokens;
        tokenizeStr(requests, tokens);
        if(tokens.size()==1){
            cout<<"...No Groups.\n";
        }else{
            cout<<"...Groups list:\n";
            for(int i=1;i<tokens.size();i++){
                cout<<tokens[i]<<endl;
            }
        }
    }else{
        cout<<"...Group listing failed.\n";
    }
}

void list_requests(string command, vector<string> tokens){
    int tr_fd = connect_tracker();
    int n;
    if(tr_fd < 0){
        cout<<"...Error: connecting to tracker\n";
        return;
    }
    command = command + " " + credentials.first;
    n = send(tr_fd,command.c_str(),command.length(), 0);
    if (n < 0){
        cout<<"...Error: sending request to  tracker!\n";
        close(tr_fd);
        return;
    }
    char buffer[4097];
    bzero(buffer,4097);
    n = read(tr_fd,buffer,4097);  
    if (n < 0){
        cout<<"...Tracker ack for {list_request} request failed!\n";
        close(tr_fd);
        return;
    }
    if(buffer[0]=='1'){
        string requests(buffer);
        vector<string> tokens;
        tokenizeStr(requests, tokens);
        if(tokens.size()==1){
            cout<<"...No pending requests.\n";
        }else{
            cout<<"...Request list:\n";
            for(int i=1;i<tokens.size();i++){
                cout<<tokens[i]<<endl;
            }
        }
    }else if(buffer[0]=='2'){
        cout<<"...You're not admin of the group\n";
    }else if(buffer[0]=='3'){
        cout<<"...Group doesnt exist\n";
    }else{
        cout<<"...Request listing failed.\n";
    }
    close(tr_fd);
    return;    
}

void leave_group(string command, vector<string> tokens){
    int tr_fd = connect_tracker();
    int n;
    if(tr_fd < 0){
        cout<<"...Error: connecting to tracker\n";
        return;
    }
    command = command + " " + credentials.first;
    n = send(tr_fd,command.c_str(),command.length(), 0);
    if (n < 0){
        cout<<"...Error: sending request to  tracker!\n";
        close(tr_fd);
        return;
    }
    char buffer[257];
    bzero(buffer,257);
    n = read(tr_fd,buffer,257);  
    if (n < 0){
        cout<<"...Tracker ack for {leave_group} request failed!\n";
        close(tr_fd);
        return;
    }
    if(buffer[0]=='1'){
        cout<<"...Group left succesfully.\n";
    }else if(buffer[0]=='2'){
        cout<<"...You're not a member of the group.\n";
    }else if(buffer[0]=='3'){
        cout<<"...Group doesn't exists\n";
    }else{
        cout<<"...Group leaving failed.\n";
    }
    close(tr_fd);
    return;    
}

void login(string command, vector<string> tokens){
    int tr_fd = connect_tracker();
    int n;
    if(tr_fd < 0){
        cout<<"...Error: connecting to tracker\n";
        return;
    }
    command = command + " " + to_string(portno);
    n = send(tr_fd,command.c_str(),command.length(), 0);
    if (n < 0){
        cout<<"...Error: sending credentials to  tracker!\n";
        close(tr_fd);
        return;
    }
    char buffer[257];
    bzero(buffer,257);
    n = read(tr_fd,buffer,257);  
    if (n < 0){
        cout<<"...Tracker ack for {login} request failed!\n";
        close(tr_fd);
        return;
    }
    if(buffer[0]=='1'){
        credentials = make_pair(tokens[1], tokens[2]);
        cout<<"...Login Sucessfull.\n";
        loggedIn = true;
    }else if(buffer[0]=='2'){
        cout<<"...User is already at some other node.\n";
    }else{
        cout<<"...Login Failed. Try again.\n";
    }
    close(tr_fd);
    return;    
}

void logout(){
    int tr_fd = connect_tracker();
    int n;
    if(tr_fd < 0){
        cout<<"...Error: connecting to tracker\n";
        return;
    }
    string command = "logout " + credentials.first;
    n = send(tr_fd,command.c_str(),command.length(), 0);
    if (n < 0){
        cout<<"...Error: sending credentials to  tracker!\n";
        close(tr_fd);
        return;
    }
    char buffer[257];
    bzero(buffer,257);
    n = read(tr_fd,buffer,257);  
    if (n < 0){
        cout<<"...Tracker ack for {login} request failed!\n";
        close(tr_fd);
        return;
    }
    if(buffer[0]=='1'){
        cout<<"...Logout Sucessfull.\n";
        loggedIn = false;
    }else{
        cout<<"...Logout Failed. User doesnt exits.\n";
    }
    close(tr_fd);
    return;   
}

void download_chunk(int chunk_no, int portno, string filepath, unordered_map<int, vector<char>> &file)
{
    int tr_fd = connect_peer(portno);
    if(tr_fd<0){
        cout<<"...Error connecting\n";
        return;
    }
    cout<<"..."<<chunk_no<<": Connected. Starting Download\n";
    string cmd = to_string(chunk_no) + " " + filepath;
    int n;
    n=send(tr_fd, cmd.c_str(), cmd.length(), 0);
    if (n < 0){
        cout<<"..."<<"Error connecting to socket.\n";
    }
    char buffer[256];
    bzero(buffer,256);
    n = read(tr_fd,buffer,255);
    int length = atoi(buffer);
    vector <char> filebuffer(length);
    int offset = 1;
    while(offset <= length){
        n=read(tr_fd, &filebuffer[offset-1], length);
        offset+=n;
    }
    n=send(tr_fd, "send hash", 10, 0);
    if (n < 0){
        cout<<"..."<<"Error writing to socket.\n";
    }
    unsigned char hash[20];
    calc_hash(filebuffer, hash);
    unsigned char hash2[20];
    bzero(hash2,20);
    n = read(tr_fd,hash2,20);
    if (n < 0){
        cout<<"..."<<"Error reading from socket.\n";
    }
    if(cmpHash(hash,hash2) == 0)
        cout<<"...Recieved"<<offset<<"bytes for chunk no"<<chunk_no<<" & hash verified"<<endl;
    else
        cout<<"...Recieved"<<offset<<"bytes for chunk no"<<chunk_no<<endl;
    file[chunk_no] = filebuffer;
    close(tr_fd);
    cout<<"..."<<chunk_no<<":closed "<<endl;
}

void select_chunks(string command, vector<string> tokens, vector<string> ports){
    //Get no of chunks
    unordered_map<int, vector<char>> file;
    vector<thread> chunkthreads;
    ports.erase(ports.begin());
    int chunks = stoi(ports[ports.size()-1]);
    ports.pop_back();
    cout<<ports.size()<<endl;
    for(int i=0; i<chunks; i++){
        string port = ports[rand() % ports.size()];
        chunkthreads.push_back(thread(download_chunk, i, stoi(port), tokens[2], ref(file) ));
    }
    for(int i=0; i<chunkthreads.size(); i++){
        chunkthreads[i].join();
    }
    int length =0;
    ofstream out(tokens[3].c_str(), ios::binary );
    for(int i=0; i<chunks; i++){
        out.write( &file[i][0], file[i].size());
        length += file[i].size();
    }
    cout<<"..."<<length<<"Bytes downloaded."<<endl;
    command = "upload_file " + tokens[2] + " " + tokens[1];
    vector<string> cmdTokens;
    tokenizeStr(command, cmdTokens);
    upload_file(command, cmdTokens); 
}

void server_handle(int clientfd)
{
    char buffer[256];
    int n;
    bzero(buffer,256);
    //Read chunk & filename
    n = read(clientfd,buffer,255);
    if (n < 0) 
        error("ERROR reading from socket");
    vector<string> tokens;
    tokenizeStr(string(buffer), tokens);
    cout<<"Chunk:"<<tokens[0]<<" File:"<<tokens[1]<<endl;
    ifstream is( tokens[1].c_str(), ios::binary);
    if (!is) // Failed to open
    {    
        cout<<"...failed to open"<<endl;
        close(clientfd);
        return;
    }
    // get length of file:
    is.seekg (0, ios::end);
    size_t l = is.tellg();
    size_t length = (size_t)min(512*1024, (int)l-(stoi(tokens[0])*1024*512));
    vector <char> filebuffer(length);
    //send length of file
    char lenarr[100];
    bzero(lenarr,100);
    sprintf(lenarr, "%1d", (int)length);
    if (n < 0) 
        error("ERROR writing to socket");
    n = send(clientfd,lenarr,255,0);
    is.seekg (stoi(tokens[0])*1024*512, ios::beg);
    cout<<"...open for reading.\n";
    size_t  reads = 0;
    do
    {
        is.read(&filebuffer[reads], length - reads);
        std::size_t amount = is.gcount();
        if (amount == 0)
        {    return; // Something went wrong and it failed to read.
        }
        reads += amount;
    } while(length != reads);
    cout<<"...reading done ";
    n = send(clientfd,&filebuffer[0],filebuffer.size(), 0);
    if (n < 0) 
        error("ERROR writing to socket");
    cout<<n<<"bytes sent. "<<endl;
    unsigned char hash[20];
    calc_hash(filebuffer, hash);
    bzero(buffer,256);
    n = read(clientfd,buffer,255);
    if (n < 0) 
        error("ERROR reading from socket");
    n = send(clientfd,hash, 20, 0);
    if (n < 0) 
        error("ERROR writing to socket");
    close(clientfd);
    return;
}

void show_downloads(string command, vector<string> tokens){
    int tr_fd = connect_tracker();
    int n;
    if(tr_fd < 0){
        cout<<"...Error: connecting to tracker\n";
        return;
    }
    //send cmd
    command = command + " " + credentials.first;
    n = send(tr_fd,command.c_str(),command.length(), 0);
    if (n < 0){
        cout<<"...Error: sending request to  tracker!\n";
        close(tr_fd);
        return;
    }
    char buffer[4096];
    bzero(buffer,4096);
    n = read(tr_fd,buffer,4095);  
    if (n < 0){
        cout<<"Tracker ack for {show_downloads} request failed!\n";
        close(tr_fd);
        return;
    }
    vector<string> downloads;
    tokenizeStr(string(buffer), downloads);
    if(buffer[0]=='1'){
        cout<<string(buffer)<<endl;
        if(downloads.size()>1){
            for(auto i: downloads){
                cout<<i<<endl;
            }
        }else{
            cout<<"...You didnt download anything yet\n";
        }
    }else{
        cout<<"...Downloads listing failed.\n";
    }
    close(tr_fd);
    return;   
}

void socser(int portno){
    int serverfd, clientfd;
    vector<thread> threadtracker;
    socklen_t clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    serverfd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverfd < 0) 
        error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(serverfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        error("ERROR on binding");
    listen(serverfd,5);
    clilen = sizeof(cli_addr);
    int i = 0;
    while (1) {
        clientfd = accept(serverfd, (struct sockaddr *) &cli_addr, &clilen);
        char host[1025];
        memset(host,0,1025);
        inet_ntop(AF_INET, &cli_addr.sin_addr, host, 1025);
        printf("...%s connected on port %d\n", host, ntohs(cli_addr.sin_port));
        if (clientfd < 0) 
            error("ERROR on accept");
        threadtracker.push_back(thread(server_handle, clientfd));
    }
    close(serverfd);
}

void stop_share(string command, vector<string> tokens){
    int tr_fd = connect_tracker();
    int n;
    if(tr_fd < 0){
        cout<<"...Error: connecting to tracker\n";
        return;
    }
    //send cmd
    command = command + " " + credentials.first;
    n = send(tr_fd,command.c_str(),command.length(), 0);
    if (n < 0){
        cout<<"Error: sending request to  tracker!\n";
        close(tr_fd);
        return;
    }
    char buffer[257];
    bzero(buffer,257);
    n = read(tr_fd,buffer,257);  
    if (n < 0){
        cout<<"...Tracker ack for {stop_share} request failed!\n";
        close(tr_fd);
        return;
    }
    if(buffer[0]=='1'){
        cout<<"...File stopped sharing succesfully.\n";
    }else if(buffer[0]=='2'){
        cout<<"...You're not part of the group.\n";
    }else if(buffer[0]=='3'){
        cout<<"...Group doesn't exists\n";
    }else{
        cout<<"...File stop sharing failed.\n";
    }
    close(tr_fd);
    return;    
}

void tokenizeStr(string str, vector<string> &tokens){
    // Used to split string around spaces.
    istringstream ss(str);
    string word; 
    while (ss >> word) 
    {   
        tokens.push_back(word);
    }
    return;
}

void upload_file(string command, vector<string> tokens){
    int tr_fd = connect_tracker();
    int n;
    if(tr_fd < 0){
        cout<<"...Error: connecting to tracker\n";
        return;
    }
    //get file size
    ifstream is( tokens[1].c_str(), ios::binary);
    if (!is) // Failed to open
    {    
        cout<<"failed to open "<<tokens[1]<<endl;
        close(tr_fd);
        return;
    }
    // get length of file:
    is.seekg (0, ios::end);
    size_t l = is.tellg();
    //calculate bitmap
    int temp = (int)l/(512*1024);
    if((int)l % (512*1024) != 0) temp++;
    string bitmap = "";
    for(int i=1; i<=temp; i++){
        bitmap += "1";
    }
    char length[100];
    bzero(length, 100);
    sprintf(length, "%1d", (int)l);
    //send cmd
    command = command + " " + credentials.first + " " + string(length) + " " + bitmap;
    n = send(tr_fd,command.c_str(),command.length(), 0);
    if (n < 0){
        cout<<"...Error: sending request to  tracker!\n";
        close(tr_fd);
        return;
    }
    char buffer[257];
    bzero(buffer,257);
    n = read(tr_fd,buffer,257);  
    if (n < 0){
        cout<<"...Tracker ack for {upload_file} request failed!\n";
        close(tr_fd);
        return;
    }
    if(buffer[0]=='1'){
        cout<<"...File uploaded succesfully.\n";
    }else if(buffer[0]=='2'){
        cout<<"...You're not part of the group.\n";
    }else if(buffer[0]=='3'){
        cout<<"...Group doesn't exists\n";
    }else{
        cout<<"...File uploading failed.\n";
    }
    close(tr_fd);
    is.close();
    return;    
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr,"Invalid no of arguments\n");
        exit(1);
    }
    fstream newfile;
    newfile.open(argv[2],ios::in);
    if (newfile.is_open()){
        string tp;
        getline(newfile, tp);
        trackerPort = stoi(tp);
        newfile.close();
    }else{
        cout<<"Error reading "<<argv[2]<<endl;
        exit(1);
    }
    trackerHost = "127.0.0.1";
    string str(argv[1]);
    replace(str.begin(), str.end(), ':', ' ');
    vector<string> array;
    stringstream ss(str);
    string temp;
    while (ss >> temp)
        array.push_back(temp);
    portno = stoi(array[1]);
    //run server thread of peer
    thread cli_ser(socser, portno);
    cli_ser.detach();
    vector<thread> cmdThreads;
    while(1){
        string command;
        vector<string> tokens;
        tokens.clear();
        string str;
        getline(cin, command);
        tokenizeStr(command, tokens);
        if(tokens.size()>0){  
            if(tokens[0] == "create_user"){
                if(tokens.size()==3 && !loggedIn){
                    cmdThreads.push_back(thread(create_user,command, tokens));
                }
            }else if(tokens[0] == "login"){
                if(tokens.size()==3 && !loggedIn){
                    cmdThreads.push_back(thread(login,command, tokens));
                }
            }else if(tokens[0] == "login-force"){
                if(tokens.size()==3){
                    cmdThreads.push_back(thread(login,command, tokens));
                }
            }else if(tokens[0] == "logout"){
                if(tokens.size()==1 && loggedIn){
                    cmdThreads.push_back(thread(logout));
                }
            }else if(tokens[0] == "create_group"){
                if(tokens.size()==2 && loggedIn){
                    cmdThreads.push_back(thread(create_group, command, tokens));
                }
            }else if(tokens[0] == "join_group"){
                if(tokens.size()==2 && loggedIn){
                    cmdThreads.push_back(thread(join_group, command, tokens));
                }
            }else if(tokens[0] == "leave_group"){
                if(tokens.size()==2 && loggedIn){
                    cmdThreads.push_back(thread(leave_group, command, tokens));
                }

            }else if(tokens[0] == "requests" && tokens[1] == "list_requests"){
                if(tokens.size()==3 && loggedIn){
                    cmdThreads.push_back(thread(list_requests, command, tokens));
                }            
            }else if(tokens[0] == "accept_request"){
                if(tokens.size()==3 && loggedIn){
                    cmdThreads.push_back(thread(accept_request, command, tokens));
                }
            }else if(tokens[0] == "list_groups"){
                if(tokens.size()==1 && loggedIn){
                    cmdThreads.push_back(thread(list_groups, command, tokens));
                }
            }else if(tokens[0] == "upload_file"){
                if(tokens.size()==3 && loggedIn){
                    cmdThreads.push_back(thread(upload_file, command, tokens));
                }
            }else if(tokens[0] == "stop_share"){
                if(tokens.size()==3 && loggedIn){
                    cmdThreads.push_back(thread(stop_share, command, tokens));
                }
            }else if(tokens[0] == "list_files"){
                if(tokens.size()==2 && loggedIn){
                    cmdThreads.push_back(thread(list_files, command, tokens));
                }
            }else if(tokens[0] == "download_file"){
                if(tokens.size()==4 && loggedIn){
                    cmdThreads.push_back(thread(download_file, command, tokens));
                }
            }else if(tokens[0] == "show_downloads"){
                if(tokens.size()==1 && loggedIn){
                    cmdThreads.push_back(thread(show_downloads, command, tokens));
                }
            }
        }
    }
}
