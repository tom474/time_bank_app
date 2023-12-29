#include "./Admin.h"

#include <iostream>

Admin::Admin() {}

Admin::Admin(string usernameVal = "", string passwordVal = "")
: User(usernameVal, passwordVal) {}

string Admin::getUsername() {
    return User::getUsername();
}

void Admin::setUsername(string usernameVal) {
    User::setUsername(usernameVal);
}

string Admin::getPassword() {
    return User::getPassword();
}

void Admin::setPassword(string passwordVal) {
    User::setPassword(passwordVal);
}

void Admin::resetPassword() {
    bool isGetMemberId = false;
    Member* targetMember = nullptr;
    // Get the target user
    while(!isGetMemberId) {
        string memberUsername = InputValidator::getString("Enter member username: ");
        vector<Member*> members = Menu::allMembers;
        // Find the target user
        for (Member* member : members) {
            if (member->getUsername() == memberUsername) {
                targetMember = member;
                isGetMemberId = true;
            }
        }
        if (targetMember == nullptr) {
            cout << "Unknow member \n";
        }
    }
    // Reset password and set IsResetPassword to true
    string newPassword = InputValidator::getString("Enter new password: ");
    targetMember->setPassword(newPassword);
    targetMember->setIsResetPassword(true);
}

Admin::~Admin() {}