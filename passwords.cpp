#include "passwords.h"

#include <iostream>
#include <cstdlib>
#include <fstream>
#include <algorithm>

ErrorValues::ErrorValues(string msg) noexcept : msg(msg) {}
ErrorValues::ErrorValues(const ErrorValues& obj) noexcept : msg(obj.msg) {}
ErrorValues::~ErrorValues() {}
const char* ErrorValues::what() const noexcept { return msg.c_str(); }

Encrypt::Encrypt(string msg) : msg(msg) { _encryption(); }
void Encrypt::_encryption(){
    for(int i = 0; i < msg.size(); i++){
        msg[i] += fault_num;
        if(msg[i] > 126) msg[i] = 32 + (msg[i] - 127);
    }
    flag_crypt = 1;
}
Encrypt::Encrypt(const Encrypt& obj) : msg(obj.msg), flag_crypt(obj.flag_crypt) {}
void Encrypt::_decryption(){
    for(int i = 0; i < msg.size(); i++){
        msg[i] -= fault_num;
        if(msg[i] < 32) msg[i] = 127 - (32 - msg[i]);
    }
    flag_crypt = 0;
}
void Encrypt::getMsg(string& msg) { _decryption(); msg = this->msg; _encryption(); }
void Encrypt::setMsg(string newMsg) { msg = newMsg; _encryption(); }
string& Encrypt::showMsg() { return msg; }

Password::Password(string name, int min_l, int max_l, string chars) : name(name), min_l(min_l), max_l(max_l), chars(chars) { srand(time(nullptr)); }
void Password::generatePassword(){
    string psw = "";
    int length = rand()%(max_l-min_l)+min_l;
    for(int i = 0; i < length; i++){
        psw += chars[rand()%chars.size()];
    }
    password.setMsg(psw);
}
Password::Password(Encrypt msg) : password(msg) {}

string Password::printPassword() { string psw; password.getMsg(psw); return psw; }

string& Password::showPassword() { return password.showMsg(); }

string& Password::showName() { return name; }

PasswordFileManager::PasswordFileManager(Password* password) : password(password) {}

/*virtual*/void PasswordFileManager::read(const string& filename)
{
    std::ifstream file(filename);
    if(file.is_open()){
        string psw = "";
        while(!file.eof()){
            string line = "";
            std::getline(file, line);
            if(line.size() < 3) break;
            password->showName() = ""; 
            psw = "";
            for(int i = 0, flag = 0; i < line.size(); i++){
                if(flag == 1) psw += line[i];
                if(line[i] == ' ') flag = 1;
                if(flag == 0) password->showName() += line[i];
            }
        }
        password->showPassword() = psw;
    }
    else throw ErrorValues("File cannot be opened");
    file.close();
}

/*virtual*/void PasswordFileManager::write(const string& filename)
{
    std::ofstream file(filename, std::ios::app);
    if(file.is_open()){
        file << password->showName() << " " << password->showPassword() << "\n";
    }
    else throw ErrorValues("File cannot be opened");
    file.close();
}

void Password::read(const string& filename){
    PasswordFileManager user(this);
    user.read(filename);
}
void Password::write(const string& filename){
    PasswordFileManager user(this);
    user.write(filename);
}

LibPasswords::LibPasswords() {}
void LibPasswords::addPassword(string name, Password psw){
    libPsw.insert(libPsw.end(), {name, psw});
}
void LibPasswords::removePassword(string name){
    libPsw.erase(libPsw.find(name));
}
Password LibPasswords::findPassword(const string& name){
    if(libPsw.find(name) != libPsw.end()) return libPsw.find(name)->second;
    throw ErrorValues("Password is not find");
}

std::map<string, Password>& LibPasswords::getLib(){
    return libPsw;
}

LibPasswordsFileManager::LibPasswordsFileManager(LibPasswords* lib) : lib(lib) {}

/*virtual*/void LibPasswordsFileManager::read(const string& filename){
    std::ifstream file(filename);
    if(file.is_open()){
        while(!file.eof()){
            string line, name = "", psw = "";
            std::getline(file, line);
            for(int i = 0, flag = 0; i < line.size(); i++){
                if(flag == 1) psw += line[i];
                if(line[i] == ' ') flag = 1;
                if(flag == 0) name += line[i];
            }
            Encrypt obj;
            obj.showMsg() = psw;
            lib->addPassword(name, Password(obj));
        }
    }
    else throw ErrorValues("File cannot be opened");
    file.close();
}
/*virtual*/void LibPasswordsFileManager::write(const string& filename){
    std::ofstream file(filename);
    if(file.is_open()){
        std::map<string, Password>::iterator it = lib->getLib().begin();
        while(it != lib->getLib().end()){
            file << (*it).first << " " << (*it).second.showPassword() << "\n";
            it++;
        }
    }
    else throw ErrorValues("File cannot be opened");
    file.close();
}

void LibPasswords::read(const string& filename){
    LibPasswordsFileManager user(this);
    user.read(filename);
}
void LibPasswords::write(const string& filename){
    LibPasswordsFileManager user(this);
    user.write(filename);
}
string LibPasswords::operator[](int size){
    if(size >= libPsw.size()) throw ErrorValues("Size must be less than size map");
    std::map<string, Password>::iterator it = libPsw.begin();
    for(int i = 0; i < libPsw.size() && i != size; i++) it++;
    return (*it).first;
}

void swap(std::map<string, Password>::iterator a, std::map<string, Password>::iterator b){
    std::map<string, Password>::iterator temp = a;
    a = b;
    b = temp;
}

void QuickSort(LibPasswords& lib, int low, int high, bool(*func)(string, string)){
    if(low > high) return;
    string p = lib[(low+high)/2];
    int i = low, j = high;
    while(i <= j){
        while(func(p, lib[i])) i++;
        while(func(lib[j], p)) j--;
        if(i <= j){
            swap(lib.libPsw.find(lib[i++]), lib.libPsw.find(lib[j--]));
        }
    }
    QuickSort(lib, low, j, func);
    QuickSort(lib, i, high, func);
}

int main(void)
{
    Password psw4("");
    psw4.read("test123.txt");
    std::cout << psw4.showName() << " " << psw4.showPassword();
    return 0;
}
