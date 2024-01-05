#include "./MemberController.h"

#include <iostream>
#include <string>
#include <iomanip>
#include <algorithm>

using std::cout;
using std::endl;
using std::string;
using std::vector;

Member* MemberController::login() {
    cout << "\n---------- Login ----------\n";
    string username = InputValidator::getString("Please enter your username: ");
    string password = InputValidator::getString("Please enter your password: ");
    Member *loggedInMember;
    vector<Member *> members = Menu::allMembers;
    for (Member *member : members) {
        if (password == member->getPassword() && username == member->getUsername()) {
            cout << "Login Successfully!\n";
            loggedInMember = member;
            MemberController::resetPassword(loggedInMember);
            return loggedInMember;
        }
    }
    cout << "Error: User does not exist!" << endl;
    return nullptr;
}

void MemberController::resetPassword(Member* member) { 
    // Check if member has a temporary password 
    if (!member->getIsResetPassword()) {
        return;
    }

    // Show request to reset password and set IsResetPassword to false
    cout << "Your password has been reset!\n"; 
    string newPassword = InputValidator::getString("Enter new password: ");
    member->setPassword(newPassword);
    member->setIsResetPassword(false);
    cout << "Change password successfully!\n";
}

vector<Member *> MemberController::searchForSupporters(Member *currentMember) {
    // Show current member's information
    cout << "--------- Your Information --------\n";
    cout << "| " << std::left << std::setw(16) << "Credit Point" << " | " << std::setw(12) << currentMember->getCreditPoint() << " |\n";
    cout << "| " << std::left << std::setw(16) << "Host Rating" << " | " << std::setw(12) << currentMember->getHostRating() << " |\n";
    cout << "| " << std::left << std::setw(16) << "City" << " | " << std::setw(12) << currentMember->getAvailableCity() << " |\n";
    cout << "-----------------------------------\n\n";

    // Show the list of suitable supporters
    vector<Member *> supporters = {};
    for (Member *supporter : Menu::allMembers) {
        bool isSuitable = false;

        // Check if the supporter is the current member
        if (supporter->getMemberId() == currentMember->getMemberId()) {
            continue;
        }

        // Check if the supporter is blocked by the current member
        if (std::find(currentMember->getBlockedUsers().begin(), currentMember->getBlockedUsers().end(), supporter->getMemberId()) != currentMember->getBlockedUsers().end()) {
            continue;
        }

        // Check if the current member is blocked by the supporter
        if (std::find(supporter->getBlockedUsers().begin(), supporter->getBlockedUsers().end(), currentMember->getMemberId()) != supporter->getBlockedUsers().end()) {
            continue;
        }

        // Check available status of the supporter
        if (supporter->getAvailableStatus() == false) {
            continue;
        }

        // Check if supporter is living at the same city with the member
        if (supporter->getAvailableCity() != currentMember->getAvailableCity()) {
            continue;
        }

        // Check if the current member has the host rating greater than the supporter's min host rating
        // And check if current member has enough credit point to book the supporter
        for (Availability *availability : supporter->getAvailability()) {
            if (availability->getMinHostRating() <= currentMember->getHostRating() && availability->getPointPerHour() <= currentMember->getCreditPoint()) {
                isSuitable = true;
                break;
            }
        }

        // If the supporter is suitable, add to the list
        if (isSuitable) {
            supporters.push_back(supporter);
        }
    }

    if (supporters.size() == 0) {
        cout << "There is no suitable supporter for you!\n";
    } else {
        // Show the list of suitable supporters
        TableGenerator::generateMemberTable("Suitable Supporters", supporters);
    }

    return supporters;
}

void MemberController::createRequest(Member *currentMember) {
    vector<Member *> supporters = MemberController::searchForSupporters(currentMember);
    if (supporters.size() == 0) {
        return;
    }

    // Choose a supporter to create a request
    bool isValidID = false;
    Member *supporter;
    while (!isValidID) {
        string supporterID = InputValidator::getString("Enter the supporter's ID that you want to create a request: ");
        for (Member *member : supporters) {
            if (member->getMemberId() == supporterID) {
                supporter = member;
                isValidID = true;
                break;
            }
        }
        if (!isValidID) {
            cout << "Invalid ID. Please enter again!\n";
        }
    }
    cout << "\n";

    // Get suitable availability of the supporter
    vector<Availability *> suitableAvailability = {};
    for (Availability *availability : supporter->getAvailability()) {
        if (availability->getMinHostRating() <= currentMember->getHostRating() && availability->getPointPerHour() <= currentMember->getCreditPoint()) {
            suitableAvailability.push_back(availability);
        }
    }

    // Check if there is no suitable availability
    if (suitableAvailability.size() == 0) {
        cout << "There is no suitable availability for you!\n";
        return;
    }

    // Show the list of suitable availability
    TableGenerator::generateAvailabilityTable("Suitable Availability", suitableAvailability);

    // Create a request
    cout << "\n---------- Create a request ----------\n";
    string requestID = IdGenerator::generateRequestId();

    // Choose a session
    Availability *selectedAvailability;
    bool isValidSession = false;
    while (!isValidSession) {
        int session = InputValidator::getInt("Enter the session that you want to create a request: ");
        if (session < 1 || session > suitableAvailability.size()) {
            cout << "Invalid session. Please enter again!\n";
            continue;
        }
        selectedAvailability = suitableAvailability[session - 1];
        isValidSession = true;
    }

    // Check the maximum hours that the current member can request
    int maxHours = currentMember->getCreditPoint() / selectedAvailability->getPointPerHour();
    cout << "You can request for " << maxHours << " hours at most!\n";

    // Get desired time
    bool isValidTime = false;
    TimePeriod *timePeriod;
    while (!isValidTime) {
        // Get desired time
        string startDate, startTime;
        string endDate, endTime;
        int startHour, endHour;
        int startMinute, endMinute;

        startDate = InputValidator::getDate("Enter your desired start date (dd/mm/yyyy): ");
        startTime = InputValidator::getTime("Enter your desired start time (hh:mm): ");
        endDate = InputValidator::getDate("Enter your desired end date (dd/mm/yyyy): ");
        endTime = InputValidator::getTime("Enter your desired end time (hh:mm): ");

        startHour = Converter::stringToInteger(startTime.substr(0, 2));
        startMinute = Converter::stringToInteger(startTime.substr(3, 2));
        endHour = Converter::stringToInteger(endTime.substr(0, 2));
        endMinute = Converter::stringToInteger(endTime.substr(3, 2));

        Time time1(startDate, startHour, startMinute);
        Time time2(endDate, endHour, endMinute);
        timePeriod = new (std::nothrow) TimePeriod(time1, time2);

        // Check if the desired time is valid (overlaps with selected availability and the requested hours is less than or equal the maximum hours)
        if (selectedAvailability->getAvailableTime()->isOverlapsWith(*timePeriod)) {
            if (timePeriod->getHourDuration() <= maxHours) {
                isValidTime = true;
            } else {
                cout << "You can only request for " << maxHours << " hours!\n";
            }
        } else {
            cout << "Invalid time. Please enter again!\n";
        }
    }
    cout << "\n";

    // Get desired skills
    vector<Skill *> desiredSkills = {};
    bool isAddSkill = true;
    while (isAddSkill) {
        bool isValidSkill = false;
        while (!isValidSkill) {
            // Show the list of supporter's available skills
            cout << "Available skills that you can request: \n";
            for (int i = 0; i < selectedAvailability->getPerformedSkills().size(); i++) {
                cout << i + 1 << ". " << selectedAvailability->getPerformedSkills()[i]->getName() << "\n";
            }

            // Choose a skill
            int choice = InputValidator::getInt("Enter a choice: ");
            if (choice < 1 || choice > selectedAvailability->getPerformedSkills().size()) {
                cout << "Invalid choice. Please enter again!\n";
                continue;
            }

            // Check if the skill is already chosen
            if (std::find(desiredSkills.begin(), desiredSkills.end(), selectedAvailability->getPerformedSkills()[choice - 1]) != desiredSkills.end()) {
                cout << "You have already chosen this skill!\n";
                break;
            }

            desiredSkills.push_back(selectedAvailability->getPerformedSkills()[choice - 1]);
            isValidSkill = true;
        }

        // Ask if the member wants to add another skill
        if (selectedAvailability->getPerformedSkills().size() > 1) {
            isAddSkill = InputValidator::getBool("Do you want to add another skill? (yes/no): ");
        }
        else {
            isAddSkill = false;
        }
    }

    // Create a request
    Request *request = new Request(requestID, currentMember->getMemberId(), supporter->getMemberId(), timePeriod, desiredSkills, requestStatus::Pending);
    currentMember->sendRequest(*request);
    supporter->receiveRequest(*request);

    cout << "Request created successfully!\n";
}

void MemberController::adjustBlockedMembersList(Member *currentMember) {
    vector<string> blockedMembersStr = currentMember->getBlockedUsers();

    vector<Member *> blockedMembers = {};
    vector<Member *> notBlockedMembers = {};
    for (Member *member : Menu::allMembers) {
        if (std::find(blockedMembersStr.begin(), blockedMembersStr.end(), member->getMemberId()) != blockedMembersStr.end()) {
            blockedMembers.push_back(member);
        } else if (member->getMemberId() != currentMember->getMemberId()) {
            notBlockedMembers.push_back(member);
        }
    }

    cout << "---------- Block/Unblock Member ----------\n";
    int choice = MenuOptionsGenerator::showMenuWithSelect("Choose an action: ", {"Block member", "Unblock member"});
    switch (choice) {
    case 1: {
        TableGenerator::generateMemberTable("Not Blocked Members", notBlockedMembers);
        bool isValidID = false;
        while (!isValidID) {
            string blockedMemberID = InputValidator::getString("Enter the member's ID that you want to block: ");
            for (Member *member : notBlockedMembers) {
                if (member->getMemberId() == blockedMemberID) {
                    currentMember->blockMember(*member);
                    isValidID = true;
                    break;
                }
            }
            if (!isValidID) {
                cout << "Invalid ID. Please enter again!\n";
            }
        }
        break;
    }
    case 2: {
        TableGenerator::generateMemberTable("Blocked Members", blockedMembers);
        bool isValidID = false;
        while (!isValidID) {
            string unblockedMemberID = InputValidator::getString("Enter the member's ID that you want to unblock: ");
            for (Member *member : blockedMembers) {
                if (member->getMemberId() == unblockedMemberID) {
                    currentMember->unblockMember(*member);
                    isValidID = true;
                    break;
                }
            }
            if (!isValidID) {
                cout << "Invalid ID. Please enter again!\n";
            }
        }
        break;

    }
    default:
        cout << "Invalid option. Please enter again!\n";
        break;
    }
}

void MemberController::manageRequests(Member* currentMember) {
    bool exitLoop = false;
    while (!exitLoop) {
        cout << "\n---------- Manage Request ----------\n";
        int choice = MenuOptionsGenerator::showMenuWithSelect(
            "Choose an action: ",
            {"Exit",
            "View your sending requests",
            "View your receiving requests",
            "Accept a receiving requests",
            "Reject a receiving requests"});
        switch (choice) {
            case 0:
                exitLoop = true;
                break;
            case 1:
                // Check if the current member has any sending request
                if (currentMember->getSendingRequest().size() == 0) {
                    cout << "You have no sending request!\n";
                } else {
                    TableGenerator::generateRequestTable("Sending Requests", currentMember->getSendingRequest());
                }
                break;
            case 2:
                // Check if the current member has any receiving request
                if (currentMember->getReceivingRequest().size() == 0) {
                    cout << "You have no receiving request!\n";
                } else {
                    TableGenerator::generateRequestTable("Receiving Requests", currentMember->getReceivingRequest());
                }
                break;
            case 3:
                MemberController::acceptRequest(currentMember);
                break;
            case 4:
                MemberController::rejectRequest(currentMember);
                break;
            default:
                cout << "Invalid option. Please enter again!\n";
                break;
        }
    }
}

void MemberController::acceptRequest(Member* currentMember) {
    // Check if the current member has any receiving request
    if (currentMember->getReceivingRequest().size() == 0) {
        cout << "You have no receiving request!\n";
        return;
    }

    // Show the list of pending requests
    vector<Request*> pendingRequests = {};
    for (Request* request : currentMember->getReceivingRequest()) {
        if (request->getStatus() == "Pending") {
            pendingRequests.push_back(request);
        }
    }
    if (pendingRequests.size() == 0) {
        cout << "You have no pending request!\n";
        return;
    }
    TableGenerator::generateRequestTable("Pending Requests", pendingRequests);

    // Choose a request to accept
    bool isValidID = false;
    Request* selectedRequest;
    while (!isValidID) {
        string requestID = InputValidator::getString("Enter the request's ID that you want to accept: ");
        for (Request* request : pendingRequests) {
            if (request->getRequestID() == requestID) {
                selectedRequest = request;
                isValidID = true;
                break;
            }
        }

        // Confirm if the current member wants to accept the request
        if (isValidID) {
            bool isConfirm = InputValidator::getBool("Are you confirm to accept this request? (yes/no): ");
            if (isConfirm) {
                selectedRequest->setStatus(requestStatus::Accepted);
                cout << "Request accepted successfully!\n";
            } else {
                cout << "Request acceptance canceled!\n";
            }
        } else {
            cout << "Invalid ID. Please enter again!\n";
        }
    }
}

void MemberController::rejectRequest(Member* currentMember) {
    // Check if the current member has any receiving request
    if (currentMember->getReceivingRequest().size() == 0) {
        cout << "You have no receiving request!\n";
        return;
    }

    // Show the list of pending requests
    vector<Request*> pendingRequests = {};
    for (Request* request : currentMember->getReceivingRequest()) {
        if (request->getStatus() == "Pending") {
            pendingRequests.push_back(request);
        }
    }
    if (pendingRequests.size() == 0) {
        cout << "You have no pending request!\n";
        return;
    }
    TableGenerator::generateRequestTable("Pending Requests", pendingRequests);

    // Choose a request to reject
    bool isValidID = false;
    Request* selectedRequest;
    while (!isValidID) {
        string requestID = InputValidator::getString("Enter the request's ID that you want to reject: ");
        for (Request* request : pendingRequests) {
            if (request->getRequestID() == requestID) {
                selectedRequest = request;
                isValidID = true;
                break;
            }
        }

        // Confirm if the current member wants to reject the request
        if (isValidID) {
            bool isConfirm = InputValidator::getBool("Are you confirm to reject this request? (yes/no): ");
            if (isConfirm) {
                selectedRequest->setStatus(requestStatus::Rejected);
                cout << "Request rejected successfully!\n";
            } else {
                cout << "Request rejection canceled!\n";
            }
        } else {
            cout << "Invalid ID. Please enter again!\n";
        }
    }
}

void MemberController::topUpCredits(Member* currentMember) {
    if (currentMember == nullptr) {
        std::cerr << "Error: Member is not logged in." << std::endl;
        return;
    }

    std::cout << "\n---------- Credit Top-Up ----------\n";
    // Show current credit point
    std::cout << "Your current credit point: " << currentMember->getCreditPoint() << std::endl;

    // Get the amount to top up
    int amount = InputValidator::getInt("Enter the amount to top up: ");

    // Verify password
    string password = InputValidator::getString("Enter your password to confirm: ");
    if (password != currentMember->getPassword()) {
        std::cout << "Incorrect password! Failed to top up your credit point!\n";
        return;
    }

    if (amount > 0) {
        currentMember->creditsTopUp(amount);
        std::cout << "Credit points successfully added! Your current credit point: " 
                  << currentMember->getCreditPoint() << std::endl;
    } else {
        std::cout << "Invalid amount! Failed to top up your credit point!" << std::endl;
    }
}

void MemberController::rateMember(Member* currentMember) {
    bool exitLoop = false;
    while (!exitLoop) {
        cout << "\n---------- Rate Member ----------\n";
        int choice = MenuOptionsGenerator::showMenuWithSelect(
            "Choose an action: ",
            {"Exit",
            "View your request history",
            "Rate and review your host",
            "Rate and review your supporter"});
        switch (choice) {
            case 0:
                exitLoop = true;
                break;
            case 1:
                MemberController::viewRequestHistory(currentMember);
                break;
            case 2:
                MemberController::rateHost(currentMember);
                break;
            case 3:
                MemberController::rateSupporter(currentMember);
                break;
            default:
                cout << "Invalid option. Please enter again!\n";
                break;
        }
    }
}

void MemberController::viewRequestHistory(Member* currentMember) {
    // Check if the current member has any request
    if (currentMember->getSendingRequest().size() == 0 && currentMember->getReceivingRequest().size() == 0) {
        cout << "You have no request!\n";
        return;
    }

    // Show the list of requests
    vector<Request*> requests = {};
    for (Request* request : currentMember->getSendingRequest()) {
        requests.push_back(request);
    }
    for (Request* request : currentMember->getReceivingRequest()) {
        requests.push_back(request);
    }
    TableGenerator::generateRequestTable("Request History", requests);
}

void MemberController::rateHost(Member* currentMember) {

}

void MemberController::rateSupporter(Member* currentMember) {

}