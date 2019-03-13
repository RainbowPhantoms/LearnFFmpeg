//
// Created by MirsFang on 2019-03-12.
//
#include <iostream>

extern "C"{
#include <libavformat/avformat.h>
}

using namespace std;
int main(){

    int version =avformat_version();
    cout<<"version:"<<version<<endl;

    return 0;
}


