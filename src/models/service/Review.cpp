#include "./Review.h"

#include <iostream>

using std::string;
using std::cout;
using std::cin;
using std::endl;

Review::Review(
    string reviewIDVal = "", 
    string reviewerIDVal = "", 
    string reviewedIDVal = "", 
    reviewType typeVal = reviewType::Supporter, 
    string commentVal = "", int ratingScoreVal = 0) 
    : reviewID(reviewIDVal), 
    reviewerID(reviewerIDVal), 
    reviewedID(reviewedIDVal), 
    type(typeVal), 
    comment(commentVal), 
    ratingScore(ratingScoreVal) {
}

string Review::getReviewID() {
    return reviewID;
}
int Review::getRatingScore() {
    return ratingScore;
}
string Review::getReviewedID() {
    return reviewedID;
}
string Review::getReviewerID() {
    return reviewerID;
}
string Review::getComment() {
    return comment;
}
reviewType Review::getType() {
    return type;
}

