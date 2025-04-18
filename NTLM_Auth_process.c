#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h> 
#include<stdbool.h>
#include<openssl/md4.h>
#include<time.h>

//Resplication of SAM FILE stored in c:\windows\system32\config
// md4("howGOODmorningTHIsisawesome") = 8e09e42c743261e755be230b534291b0

#define Client_computer_size 64
#define Hash_size 16
#define Max_Users 2

struct UserInfo{
    //Format of User's info saved in sam file
    char name[32];
    char id[4];
    char LM_Hash[16];
    unsigned char NTML_Hash[MD4_DIGEST_LENGTH];
};

struct SAM{
   struct UserInfo *users[Max_Users]; // there can be only 2 Users in the system (my program my rule)  
}sam;

struct UserInfo* LoadUserInfo(char*name,char*id,char*Hash1){
    unsigned char * md4_hash = malloc(MD4_DIGEST_LENGTH);
    char original_password[] = "howGOODmorningTHIsisawesome";// for now its hardcoded but should not be for the sake of esay readablity !
    original_password[strlen(original_password)] = 0 ;
    MD4((unsigned char *)original_password,strlen(original_password),md4_hash);
    struct UserInfo*  userInfo = malloc(sizeof(struct UserInfo));
    memcpy(userInfo->name,name,strlen(name));
    memcpy(userInfo->id,id,strlen(id));
    memcpy(userInfo->LM_Hash,Hash1,strlen(Hash1));
    memcpy(userInfo->NTML_Hash,md4_hash,MD4_DIGEST_LENGTH);
    return userInfo;
}


void __attribute__((constructor)) LoadUsers(){
    sam.users[0]= LoadUserInfo("Sheila","1001","NO PASSWORD*****");
    sam.users[1]= LoadUserInfo("Umar","1002","NO PASSWORD*****");
}


struct Client_computer{
    char Username[32];  //LIMIT TO ONLY 32 LEN FOR USERNAME AND PASSWORD
    char Password[32];
};

char* GenerateRandom16BytesString() {
    char* nonce = malloc(17); // 16 chars + null terminator
    if (!nonce) return NULL; // check malloc

    const int min = 32, max = 126;

    for (int i = 0; i < 16; i++) {
        nonce[i] = (char)(rand() % (max - min + 1) + min);
    }
    nonce[16] = '\0'; // null-terminate
    return nonce;
}


struct Client_computer * client_computer(){
    struct Client_computer *info = malloc(Client_computer_size);
    //taking user's Input 
    printf("Enter Username: ");
    fgets(info->Username,sizeof(info->Username),stdin);
    info->Username[strcspn(info->Username,"\n")] =0;
    printf("Enter Password: ");
    fgets(info->Password,sizeof(info->Password),stdin);
    info->Password[strcspn(info->Password,"\n")] = 0;
    
    //converting users Password Into MD4-Hash
    unsigned char MD4_Password[MD4_DIGEST_LENGTH];
    MD4((unsigned char *)info->Password,strlen(info->Password),MD4_Password);
    memcpy(info->Password,MD4_Password,MD4_DIDEST_LENGTH);
    return info;
}

char * Encryption(const unsigned char* Hash_password,char* nonce){
    //Real encryption doesn't look like for the sake of easiness i Have xored each bytes of nonce with hash=password

    size_t i = 0;
    char *encrypted = malloc(Hash_size);
    while(i<Hash_size){
        encrypted[i]= Hash_password[i] ^ nonce[i];
	i++;
}
    return encrypted;
}



bool domain_controller(char* Username,unsigned char* Hash_password, char*nonce){

    bool User_check = false;
    size_t current_users = 0;
    while(current_users!=Max_Users){
        //checking if the username exist or not
        if(!strncmp(Username,sam.users[current_users]->name,32)){
            User_check = true;
            break;
        }else{
            current_users++;
        }
    }
    if(User_check){
    //if Username exist then we verify its password 
    char * Encryption1 = Encryption(Hash_password,nonce);
	char * Encryption2 = Encryption(sam.users[current_users]->NTML_Hash,nonce);
	if(memcmp(Encryption1,Encryption2,Hash_size)==0){
            return true;
        }else{
	        printf("Password Incorrect!");
	        exit(0);
        }

    }else{
        return 0;
    }
    return 0;
}

int main(){
    //First we take the input from user
    struct Client_computer * User = client_computer();
    // simple bool value check  
    if(domain_controller(User->Username,User->Password,Generatenonceing())){
        printf("WELCOME %s",User->Username);
    }else{
        printf("User Not found!");
    }
    return 0;
}
