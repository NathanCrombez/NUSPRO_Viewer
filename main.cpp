#include <iostream>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>


using namespace std;
using namespace cv;
using namespace boost;


int main(int argc, char** argv ){
    cout<<"\033[32m***************************************************************"<<endl;
    cout<<"***************************************************************"<<endl;
    cout<<"NUS_PRO Datasets Viewer"<<endl;
    cout<<"Use :  ./NUSPRO_Viewer \"Path/to/Dataset/folder\""<<endl;
    cout<<"Controls : "<<endl;
    cout<<"\t n : Next frame"<<endl;
    cout<<"\t p : Previous frame"<<endl;
    cout<<"\t d : Draw ground truth ON/OFF"<<endl;
    cout<<"\t i : Display infos ON/OFF"<<endl;
    cout<<"\t c : Change color"<<endl;
    cout<<"\t SPACE : Lecture auto ON/OFF"<<endl;
    cout<<"\t ESC : Quit"<<endl;
    cout<<"***************************************************************"<<endl;
    cout<<"Author : Nathan Crombez"<<endl;
    cout<<"Le2i - Laboratoire Electronique, Informatique et Image"<<endl;
    cout<<"***************************************************************\033[0m"<<endl;

    if(argc<=1){
        cout<<"\033[31mYou forgot the folder path !"<<endl;
        cout<<"./NUSPRO_Viewer \"Path/to/Dataset/folder\"\033[0m"<<endl;
        return 1;
    }

    if( !boost::filesystem::exists(lexical_cast<string>(argv[1])+"/groundtruth.txt") || !boost::filesystem::exists(lexical_cast<string>(argv[1])+"/occlusion.txt")){
        cout<<"\033[31mCan't find the data."<<endl;
        cout<<"Verify your folder path.\033[0m"<<endl;
        return 1;
    }

    ifstream ficGroundTruth, ficOcclusion;
    ficGroundTruth.open((lexical_cast<string>(argv[1])+"/groundtruth.txt").c_str());
    ficOcclusion.open((lexical_cast<string>(argv[1])+"/occlusion.txt").c_str());


    string line;
    Mat frame;
    filesystem::path sequencePath(argv[1]);
    vector<Rect> BoundingBox;
    vector<bool> Occlusion;
    bool occluded;
    Rect rect;
    Point p1,p2;
    vector<filesystem::path> FramePath;
    copy(filesystem::directory_iterator(sequencePath), filesystem::directory_iterator(), back_inserter(FramePath));
    sort(FramePath.begin(), FramePath.end());
    vector<filesystem::path>::iterator FrameIt = FramePath.begin();
    vector<Rect>::iterator BoundingBoxIt;
    vector<bool>::iterator OcclusionIt;
    do{
        if ((*FrameIt).string().find(".jpg") == std::string::npos) {
            FramePath.erase(FrameIt);
        }else{
            ficGroundTruth>> p1.x;
            ficGroundTruth>> p1.y;
            ficGroundTruth>> p2.x;
            ficGroundTruth>> p2.y;
            rect.x = p1.x;
            rect.y = p1.y;
            rect.width = p2.x - p1.x;
            rect.height = p2.y - p1.y;
            BoundingBox.push_back(rect);
            ficOcclusion>> occluded;
            if(occluded){
                Occlusion.push_back(1);
            }else{
                Occlusion.push_back(0);
            }
            FrameIt++;
        }
    }while (FrameIt != FramePath.end());


    int colorIdx=0;
    vector<Scalar> colors(5);
    colors[0] = Scalar(0,255,0);
    colors[1] = Scalar(255,0,0);
    colors[2] = Scalar(0,0,255);
    colors[3] = Scalar(255,0,255);
    colors[4] = Scalar(255,255,0);



    namedWindow("Visual tracking - Ground Truth", WINDOW_AUTOSIZE);
    FrameIt = FramePath.begin();
    BoundingBoxIt = BoundingBox.begin();
    OcclusionIt = Occlusion.begin();
    char key;
    bool automode=0, drawBox=1, drawInfo=1;
    do{
        frame = imread((*FrameIt).c_str(), 1);
        if(drawBox){
            if(*OcclusionIt)
                rectangle(frame, (*BoundingBoxIt), colors[colorIdx],1);
            else
                rectangle(frame, (*BoundingBoxIt), colors[colorIdx],2);
        }

        if(drawInfo){
        putText(frame, lexical_cast<string>((*FrameIt).filename())+" - ("+lexical_cast<string>(distance(FramePath.begin(),FrameIt)) + " / " +lexical_cast<string>(distance(FramePath.begin(),FramePath.end())) + ")" ,Point(25,50), FONT_HERSHEY_SIMPLEX, 0.5, colors[colorIdx]);
        putText(frame, "Ground truth : "+lexical_cast<string>((*BoundingBoxIt).x) + " " + lexical_cast<string>((*BoundingBoxIt).y) + " " + lexical_cast<string>((*BoundingBoxIt).width) + " "+ lexical_cast<string>((*BoundingBoxIt).height),Point(25,70), FONT_HERSHEY_SIMPLEX, 0.5, colors[colorIdx]);
        if(*OcclusionIt)
            putText(frame, "Occlusion", Point(25,90), FONT_HERSHEY_SIMPLEX, 0.5, colors[colorIdx]);
        else
            putText(frame, "No occlusion", Point(25,90), FONT_HERSHEY_SIMPLEX, 0.5, colors[colorIdx]);
        }


        imshow( "Visual tracking - Ground Truth", frame );

        if(!automode){
            key = waitKey(0);
            //cout<<(int)key<<endl;
            switch((int)key){
            case 110 :   //NEXT
                if(FrameIt!=FramePath.end()-1){
                    FrameIt++;
                    BoundingBoxIt++;
                    OcclusionIt++;
                }break;
            case 112 :   //PREVIOUS
                if(FrameIt!=FramePath.begin()){
                    FrameIt--;
                    BoundingBoxIt--;
                    OcclusionIt--;
                }break;
            case 27 :   //QUIT
                FrameIt = FramePath.end();break;
            case 105 : //INFOS
                drawInfo=!drawInfo;break;
            case 32 : //LECTURE MODE
                automode=!automode;break;
            case 100 : //DRAW MODE
                drawBox=!drawBox;break;
            case 99 : //DRAW MODE
               if(++colorIdx>colors.size()-1)
                    colorIdx=0;
               break;
            }
        }else{
            FrameIt++;
            BoundingBoxIt++;
            OcclusionIt++;
            key = waitKey(100);
            switch((int)key){
            case 27 :   //QUIT
                FrameIt = FramePath.end();break;
            case 105 : //INFOS
                drawInfo=!drawInfo;break;
            case 32 : //LECTURE MODE
                automode=!automode;break;
            case 100 : //DRAW MODE
                drawBox=!drawBox;break;
            case 99 : //DRAW MODE
                if(++colorIdx>colors.size()-1)
                    colorIdx=0;
               break;
            }
        }
    }while (FrameIt != FramePath.end());



    return 0;
}
