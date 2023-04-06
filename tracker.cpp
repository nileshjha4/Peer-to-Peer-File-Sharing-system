#include <bits/stdc++.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread>
#include <pthread.h>
using namespace std;

int portno;
unordered_map <string,string> userCredential;

void error(const char *msg);

class Peers{
public:
    string username;
    int port;
    bool loggedIn;
    set<string> groups;
    unordered_map<string, string> downloads;
    //key = filepath, value = groupid
    Peers(string user, int p){
        username = user;
        port = p;
        loggedIn = true;
    }

    string getDownloads(){
        string str = "";
        for(auto i: downloads){
            str = str + "[D][" + i.second + "]" + i.first + " ";
        }
        cout<<str<<endl;
        return str;
    }
};
unordered_map<string, Peers> peertracker;

class Group{
public:
    string groupid;
    string admin;
    set<string> request;
    unordered_map<string, unordered_map<string, bool>>clients;
    //key=clientid, value=vector of files shared by client
    unordered_map<string, unordered_map<string,pair<string,string>> >files; 
    //key.first=filename, value.key=clientid, value.key.first=filesize, value.key.second=bitmap
    Group(string id, string user){
        groupid = id;
        admin = user;
    }
    void addrequest(string clientid){
        request.insert(clientid);
    }
    void addFile(string filepath, string clientid, string bitmap, string filesize){
        //Do something
        clients[clientid].emplace(filepath, true);
        files[filepath][clientid] = make_pair(filesize,bitmap);

    }
    void deleteClient(string clientid){
        for(auto i: clients[clientid]){
            files.at(i.first).erase(clientid);
        }
        clients.erase(clientid);
    }
    string showFiles(){
        string str="";
        for(auto i: files){
            str= str + "\n" + i.first + ":\n" ;
            for(auto j: i.second){
                cout<<j.first<<" "<<i.first<<" "<<clients[j.first][i.first]<<endl;
                if(clients[j.first][i.first] == true){
                    str = str + j.first + " " ;
                }
            }
        }
        cout<<str;   
        return str; 
    }   

    string getPorts(string filename){
        string str="";
        int k;
        for(auto i: files[filename]){
            if(clients[i.first][filename] == true){    
                str = str + to_string(peertracker.at(i.first).port) + " ";
            }
            k=i.second.second.size();
        } 
        str = str + " " + to_string(k);
        cout<<str<<endl;
        return str; 
    }

    void showClients(){
        string str=  groupid + ": ";
        for(auto i: clients){
            str= str + i.first + " ";
        }
        cout<<str<<endl;
    }

    void stopShare(string clientid, string filename){
        for(auto i : clients[clientid]){
            if(i.first == filename)
                clients[clientid][filename] = false;
        }
    }

    void stopShareAll(string clientid){
        for(auto i : clients[clientid]){
            clients[clientid][i.first] = false;
        }
    }

    void startShareAll(string clientid){
        for(auto i : clients[clientid]){
            clients[clientid][i.first] = true;
        }
    }    


};
unordered_map<string, Group> grouptracker;

void error(const char *msg){
    perror(msg);
    exit(1);
}

void printgroup(){
    for(auto i: grouptracker){
        cout<<i.first<<endl;
    }
}
void printpeer(){
    for(auto i: peertracker){
        cout<<i.first<<endl;
    }
}

void tokenizeStr(string str, vector<string> &tokens){
    // Used to split string around spaces.
    istringstream ss(str);
    string word; 
    while (ss >> word) 
    {
        tokens.push_back(word);
    }
}

void printusers(){
    for(auto i: peertracker){
        cout<<i.second.username<<"@"<<i.second.port<<endl;
    }
}

void server_handle(int clientfd)
{
    char buffer[1024];
    int n;
    bzero(buffer,1024);
    n = read(clientfd,buffer,1024);
    if (n < 0) 
        cout<<"ERROR reading from socket\n";
    vector<string> tokens;
    tokenizeStr(string(buffer), tokens);
    if(tokens[0] == "create_user"){
        if(userCredential.find(tokens[1]) == userCredential.end()){
            userCredential[tokens[1]] = tokens[2];
            Peers temp(tokens[1], stoi(tokens[3])); 
            peertracker.emplace(tokens[1], temp);
            n=send(clientfd, "1" , 2, 0);
            printpeer();
        }else{
            n=send(clientfd, "0" , 2, 0);
        }
        if (n < 0) cout<<"ERROR writing acknowledgement\n";
    }else if(tokens[0] == "login"){
        if(userCredential.find(tokens[1]) != userCredential.end()){
            if(userCredential[tokens[1]] == tokens[2]){
                if(peertracker.at(tokens[1]).loggedIn){
                    n=send(clientfd, "2", 2, 0);
                }else{
                    peertracker.at(tokens[1]).loggedIn = true;
                    peertracker.at(tokens[1]).port = stoi(tokens[3]);
                    for(auto i: peertracker.at(tokens[1]).groups){
                        grouptracker.at(i).startShareAll(tokens[1]);
                    }
                    n=send(clientfd, "1" , 2, 0);
                }
            }else{
                n=send(clientfd, "0", 2, 0);
            }
        }
        n=send(clientfd, "0" , 2, 0);
        if (n < 0) cout<<"ERROR writing acknowledgement\n";
    }else if(tokens[0] == "logout"){
        if(userCredential.find(tokens[1]) != userCredential.end()){
            if(peertracker.at(tokens[1]).loggedIn == false){
                n=send(clientfd, "2", 2, 0);
            }else{
                peertracker.at(tokens[1]).loggedIn = false;
                for(auto i: peertracker.at(tokens[1]).groups){
                    grouptracker.at(i).stopShareAll(tokens[1]);
                }
                n=send(clientfd, "1" , 2, 0);
            }
        }else{
            n=send(clientfd, "0" , 2, 0);
        }
        if (n < 0) cout<<"ERROR writing acknowledgement\n";
    }else if(tokens[0] == "create_group"){
        if(peertracker.find(tokens[2]) != peertracker.end() && peertracker.at(tokens[2]).loggedIn == true){
            if(grouptracker.find(tokens[1]) == grouptracker.end()){
                Group temp(tokens[1],tokens[2]);
                grouptracker.emplace(tokens[1], temp);
                grouptracker.at(tokens[1]).clients.emplace(tokens[2],unordered_map<string,bool>{});
                peertracker.at(tokens[2]).groups.insert(tokens[1]);
                cout<<grouptracker.at(tokens[1]).groupid<<" created by "<<grouptracker.at(tokens[1]).admin<<endl;
                n=send(clientfd, "1", 2, 0);
                printgroup();
                //grouptracker.at(tokens[1]).showClients();
            }else{
                n=send(clientfd, "2", 2, 0);//Group already exist
            }
        }else{
            n=send(clientfd, "0", 2, 0);
        }
    }else if(tokens[0] == "join_group"){
        if(peertracker.find(tokens[2]) != peertracker.end() && peertracker.at(tokens[2]).loggedIn == true){
            if(grouptracker.find(tokens[1]) != grouptracker.end()){
                if(grouptracker.at(tokens[1]).clients.find(tokens[2]) == grouptracker.at(tokens[1]).clients.end()){
                    grouptracker.at(tokens[1]).addrequest(tokens[2]);
                    n=send(clientfd, "1", 2, 0);
                }else{
                    n=send(clientfd, "2", 2, 0);//Already part of group
                }
            }else{
                n=send(clientfd, "3", 2, 0);//Group doesnt exists
            }
        }else{
            n=send(clientfd, "0", 2, 0);
        }
    }else if(tokens[0] == "leave_group"){
        if(peertracker.find(tokens[2]) != peertracker.end() && peertracker.at(tokens[2]).loggedIn == true){
            if(grouptracker.find(tokens[1]) != grouptracker.end()){    
                if(grouptracker.at(tokens[1]).clients.find(tokens[2]) != grouptracker.at(tokens[1]).clients.end()){
                    grouptracker.at(tokens[1]).showClients();
                    grouptracker.at(tokens[1]).deleteClient(tokens[2]);
                    n=send(clientfd, "1", 2, 0);
                    grouptracker.at(tokens[1]).showClients();
                }else{
                    n=send(clientfd, "2", 2, 0);//You're not member of the group
                }
            }else{
                n=send(clientfd, "3", 2, 0);//Group doesnt exists
            }
        }else{
            n=send(clientfd, "0", 2, 0);
        }
    }else if(tokens[0] == "requests" && tokens[1] == "list_requests"){
        if(peertracker.find(tokens[3]) != peertracker.end() && peertracker.at(tokens[3]).loggedIn == true){
            if(grouptracker.find(tokens[2]) != grouptracker.end()){
                if(grouptracker.at(tokens[2]).admin == tokens[3]){
                    string req = "1 ";
                    for(auto i: grouptracker.at(tokens[2]).request){
                        req = req + i + " ";
                    }
                    cout<<req<<endl;
                    n=send(clientfd, req.c_str(), req.length(), 0);
                }else{
                    n=send(clientfd, "2", 2, 0);//You're not admin
                }
            }else{
                n=send(clientfd,"3",2,0);//Group doesnt exist
            }
        }else{
            n=send(clientfd,"0",2,0);
        }
    }else if(tokens[0] == "list_groups"){
        string req = "1 ";
        for(auto i: grouptracker){
            req = req + i.second.groupid + " ";
        }
        cout<<req<<endl;
        n=send(clientfd, req.c_str(), req.length(), 0);
    }else if(tokens[0] == "accept_request"){
        if(peertracker.find(tokens[2]) != peertracker.end() && peertracker.at(tokens[2]).loggedIn == true){
            if(peertracker.find(tokens[3]) != peertracker.end() && peertracker.at(tokens[3]).loggedIn == true){   
                if(grouptracker.find(tokens[1]) != grouptracker.end()){
                    if(grouptracker.at(tokens[1]).admin == tokens[3]){
                        grouptracker.at(tokens[1]).request.erase(tokens[2]);
                        grouptracker.at(tokens[1]).clients.emplace(tokens[2],unordered_map<string,bool>{});
                        peertracker.at(tokens[2]).groups.insert(tokens[1]);
                        n=send(clientfd,"1",2,0);
                        grouptracker.at(tokens[1]).showClients();
                    }else{
                        n=send(clientfd, "2", 2, 0);//You're not authorised
                    }
                }else{
                    n=send(clientfd,"3",2,0);//Group doesnt exist
                }
            }else{
                n=send(clientfd,"4",2,0);
            }
        }else{
            n=send(clientfd,"0",2,0);
        }
    }else if(tokens[0] == "upload_file"){
        if(peertracker.find(tokens[3]) != peertracker.end() && peertracker.at(tokens[3]).loggedIn == true){
            if(grouptracker.find(tokens[2]) != grouptracker.end()){
                if(grouptracker.at(tokens[2]).clients.find(tokens[3]) != grouptracker.at(tokens[2]).clients.end()){
                    //upload file
                    grouptracker.at(tokens[2]).addFile(tokens[1],tokens[3], tokens[5], tokens[4]);
                    n=send(clientfd, "1", 2, 0);
                }else{
                    n=send(clientfd, "2", 2, 0);//Not part of group
                }
            }else{
                n=send(clientfd, "3", 2, 0);//Group doesnt exists
            }
        }else{
            n=send(clientfd,"0",2,0);
        }
    }else if(tokens[0] == "stop_share"){
        if(peertracker.find(tokens[3]) != peertracker.end() && peertracker.at(tokens[3]).loggedIn == true){
            if(grouptracker.find(tokens[2]) != grouptracker.end()){
                if(grouptracker.at(tokens[2]).clients.find(tokens[3]) != grouptracker.at(tokens[2]).clients.end()){
                    grouptracker.at(tokens[2]).stopShare(tokens[3],tokens[1]);
                    n=send(clientfd, "1", 2, 0);
                }else{
                    n=send(clientfd, "2", 2, 0);//Not part of group
                }
            }else{
                n=send(clientfd, "3", 2, 0);//Group doesnt exists
            }
        }else{
            n=send(clientfd,"0",2,0);
        }
    }else if(tokens[0] == "list_files"){
        if(peertracker.find(tokens[2]) != peertracker.end() && peertracker.at(tokens[2]).loggedIn == true){
            if(grouptracker.find(tokens[1]) != grouptracker.end()){
                if(grouptracker.at(tokens[1]).clients.find(tokens[2]) != grouptracker.at(tokens[1]).clients.end()){
                    string temp = grouptracker.at(tokens[1]).showFiles();
                    temp = "1 " + temp; 
                    n=send(clientfd, temp.c_str() , temp.length(), 0);
                }else{
                    n=send(clientfd, "2", 2, 0);//Not part of group
                }
            }else{
                n=send(clientfd, "3", 2, 0);//Group doesnt exists
            }
        }else{
            n=send(clientfd,"0",2,0);
        }
    }else if(tokens[0]=="download_file"){
        if(peertracker.find(tokens[4]) != peertracker.end() && peertracker.at(tokens[4]).loggedIn == true){
            if(grouptracker.find(tokens[1]) != grouptracker.end()){
                if(grouptracker.at(tokens[1]).clients.find(tokens[4]) != grouptracker.at(tokens[1]).clients.end()){
                    if(grouptracker.at(tokens[1]).files.find(tokens[2]) != grouptracker.at(tokens[1]).files.end()){
                        if(grouptracker.at(tokens[1]).clients[tokens[4]].find(tokens[2]) == grouptracker.at(tokens[1]).clients[tokens[4]].end()){
                            string temp = grouptracker.at(tokens[1]).getPorts(tokens[2]);
                            temp = "1 " + temp; 
                            peertracker.at(tokens[4]).downloads.emplace(tokens[2],tokens[1]);
                            n=send(clientfd, temp.c_str() , temp.length(), 0);
                        }else{
                            n=send(clientfd, "5", 2, 0); //Already seeding
                        }
                    }else{
                        n=send(clientfd, "4", 2, 0);//File doesnt exists
                    }
                }else{
                    n=send(clientfd, "2", 2, 0);//Not part of group
                }
            }else{
                n=send(clientfd, "3", 2, 0);//Group doesnt exists
            }
        }else{
            n=send(clientfd,"0",2,0);
        }
    }else if(tokens[0]=="show_downloads"){
        if(peertracker.find(tokens[1]) != peertracker.end() && peertracker.at(tokens[1]).loggedIn == true){
            //Do something
            string temp = peertracker.at(tokens[1]).getDownloads();
            temp = "1 " + temp;
            n=send(clientfd, temp.c_str() , temp.length(), 0);
        }else{
            n=send(clientfd,"0",2,0);
        }
    }
    close(clientfd);
    return;
}

int main(int argc, char *argv[])
{
    vector<thread> sReqTh;
    int serverfd, clientfd;
    socklen_t clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    if (argc != 3) {
        cout<<"Invalid no of arguments.\n"<<endl;
        exit(1);
    }
    fstream newfile;
    newfile.open(argv[2],ios::in);
    if (newfile.is_open()){
        string tp;
        getline(newfile, tp);
        portno = stoi(tp);
        newfile.close();
    }else{
        cout<<"Error reading "<<argv[2]<<endl;
        exit(1);
    }

    //Open Server Socket
    serverfd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverfd < 0) error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(serverfd, (struct sockaddr *) &serv_addr,
            sizeof(serv_addr)) < 0) 
            error("ERROR on binding");
    listen(serverfd,10);
    clilen = sizeof(cli_addr);
    int i = 0;
 
    while (1) {
        //Accept 
        cout<<">>";
        clientfd = accept(serverfd, (struct sockaddr *) &cli_addr, &clilen);
        char host[1025];
        memset(host,0,1025);
        inet_ntop(AF_INET, &cli_addr.sin_addr, host, 1025);
        if (clientfd < 0) 
            error("ERROR on accept");
        printf("%s:%d connected\n", host, ntohs(cli_addr.sin_port));
        sReqTh.push_back(thread(server_handle, clientfd));
        //sReqTh[sReqTh.size()-1].detach();
    }
    close(serverfd);
    return 0; 
}
