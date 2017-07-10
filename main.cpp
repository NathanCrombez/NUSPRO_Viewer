#include <iostream>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/progress.hpp>



using namespace std;
using namespace cv;
using namespace boost;


int main(int argc, char** argv ){
    cout<<"\033[32m***************************************************************"<<endl;
    cout<<"***************************************************************"<<endl;
    cout<<"NUS_PRO Datasets Viewer"<<endl;
    cout<<"Use :  ./NUSPRO_Viewer --help --mode {m} --path {\"/Path/to/Dataset/folder\"}"<<endl;
    cout<<"Modes : "<<endl;
    cout<<"- Viewer mode m=0 (default) : "<<endl;
    cout<<"\tControls :"<<endl;
    cout<<"\t  n : Next frame"<<endl;
    cout<<"\t  p : Previous frame"<<endl;
    cout<<"\t  d : Draw ground truth ON/OFF"<<endl;
    cout<<"\t  m : Increase draw delay"<<endl;
    cout<<"\t  l : Decrease draw delay"<<endl;
    cout<<"\t  i : Display infos ON/OFF"<<endl;
    cout<<"\t  c : Change color"<<endl;
    cout<<"\t  SPACE : Lecture auto ON/OFF"<<endl;
    cout<<"\t  ESC : Quit"<<endl;
    cout<<"- Extract mode m=1 : "<<endl;
    cout<<"\tExtract the ground truth images."<<endl;
    cout<<"***************************************************************"<<endl;
    cout<<"Author : Nathan Crombez"<<endl;
    cout<<"Le2i - Laboratoire Electronique, Informatique et Image"<<endl;
    cout<<"***************************************************************\033[0m"<<endl;


    program_options::options_description desc("Allowed options");
    desc.add_options()
            ("help", "Describe arguments")
            ("mode", program_options::value<int>(), "0: viewer(default) / 1: extract")
            ("path", program_options::value<string>(), "Path to the data folder");


    program_options::variables_map vm;
    program_options::store(program_options::parse_command_line(argc, argv, desc), vm);
    program_options::notify(vm);

    if (vm.count("help")) {
        cout << desc << endl;
        return 1;
    }

    int mode=0;
    string folderPath;

    if(vm.count("path")){
        folderPath = vm["path"].as<string>();
    }else{
        cout<<"\033[31mDid you forget the folder path ?\033[0m"<<endl;
        return 1;
    }
    if( !boost::filesystem::exists(folderPath+"/groundtruth.txt") || !boost::filesystem::exists(folderPath+"/occlusion.txt")){
        cout<<"\033[31mCan't find the data."<<endl;
        cout<<"Verify your folder path.\033[0m"<<endl;
        return 1;
    }

    if (vm.count("mode")) {
        mode = vm["mode"].as<int>();
        switch (mode) {
        case 0:
            cout<<"Mode : Viewer"<<endl;
            break;
        case 1:
            cout<<"Mode : Extract"<<endl;
            filesystem::create_directory(folderPath+"/GroundTruthROI");
            break;
        default:
            cout<<"\033[31mWhat is the mode : "<<mode<<"\033[0m"<<endl;
            cout << desc << endl;
            return 1;
            break;
        }
    }else{
        cout<<"Mode : Viewer (default)"<<endl;
    }

    ifstream ficGroundTruth, ficOcclusion;
    ficGroundTruth.open((folderPath+"/groundtruth.txt").c_str());
    ficOcclusion.open((folderPath+"/occlusion.txt").c_str());


    string line;
    Mat frame;
    filesystem::path sequencePath(folderPath);
    vector<Rect> BoundingBox;
    vector<int> Occlusion;
    int occluded;
    Rect rect;
    Point p1,p2;
    vector<filesystem::path> FramePath;
    copy(filesystem::directory_iterator(sequencePath), filesystem::directory_iterator(), back_inserter(FramePath));
    sort(FramePath.begin(), FramePath.end());
    vector<filesystem::path>::iterator FrameIt = FramePath.begin();
    vector<Rect>::iterator BoundingBoxIt;
    vector<int>::iterator OcclusionIt;
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
            Occlusion.push_back(occluded);

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
    int drawDelay=1, drawDelayMax=10;
    Mat GroundTruthROI;

    progress_display progressBarExtractROI(FramePath.size());

    do{
        frame = imread((*FrameIt).c_str(), 1);
        if(mode==1){ //extract mode
            GroundTruthROI = frame((*BoundingBoxIt));
            imwrite(folderPath+"/GroundTruthROI/"+(*FrameIt).filename().c_str(),GroundTruthROI);
            FrameIt++;
            BoundingBoxIt++;
            OcclusionIt++;
            ++progressBarExtractROI;
        }else if(mode==0){  //viewer mode (default)
            if(drawBox){
                for(int i=0;i<drawDelay;i++){
                    if(i<=distance(BoundingBox.begin(),BoundingBoxIt)){
                        if(i==0)
                            rectangle(frame, *(BoundingBoxIt-i), colors[colorIdx],2);
                        else
                            rectangle(frame, *(BoundingBoxIt-i), colors[colorIdx]-Scalar(colors[colorIdx][0]/(i+1),colors[colorIdx][1]/(i+1),colors[colorIdx][2]/(i+1)),1);
                    }
                }
            }

            if(drawInfo){
                putText(frame, lexical_cast<string>((*FrameIt).filename())+" - ("+lexical_cast<string>(distance(FramePath.begin(),FrameIt)) + " / " +lexical_cast<string>(distance(FramePath.begin(),FramePath.end())) + ")" ,Point(25,50), FONT_HERSHEY_SIMPLEX, 0.5, colors[colorIdx]);
                putText(frame, "Ground truth : "+lexical_cast<string>((*BoundingBoxIt).x) + " " + lexical_cast<string>((*BoundingBoxIt).y) + " " + lexical_cast<string>((*BoundingBoxIt).width) + " "+ lexical_cast<string>((*BoundingBoxIt).height),Point(25,70), FONT_HERSHEY_SIMPLEX, 0.5, colors[colorIdx]);
                if((*OcclusionIt)==2)
                    putText(frame, "Full Occlusion", Point(25,90), FONT_HERSHEY_SIMPLEX, 0.5, colors[colorIdx]);
                else if ((*OcclusionIt)==1)
                    putText(frame, "Partial Occlusion", Point(25,90), FONT_HERSHEY_SIMPLEX, 0.5, colors[colorIdx]);
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
                        ++progressBarExtractROI;
                    }break;
                case 112 :   //PREVIOUS
                    if(FrameIt!=FramePath.begin()){
                        FrameIt--;
                        BoundingBoxIt--;
                        OcclusionIt--;
                        cout<<"\33[2K \033[A \33[2K \033[A \33[2K \033[A"; //Ugly and not cross-platform but it works so : hey ! who care !
                        int currProgBar = progressBarExtractROI.count();
                        progressBarExtractROI.restart(FramePath.size());
                        progressBarExtractROI+=currProgBar-1;
                    }break;
                case 27 :   //QUIT
                    FrameIt = FramePath.end();break;
                case 105 : //INFOS
                    drawInfo=!drawInfo;break;
                case 32 : //LECTURE MODE
                    automode=!automode;break;
                case 100 : //DRAW MODE
                    drawBox=!drawBox;break;
                case 99 : //CHANGE COLOR
                    if(++colorIdx>colors.size()-1)
                        colorIdx=0;
                    break;
                case 109 : //CHANGE DELAY
                    if(drawDelay+1<drawDelayMax)
                        drawDelay++;
                    break;
                case 108 : //CHANGE DELAY
                    if(drawDelay-1>0)
                        drawDelay--;
                    break;
                }
            }else{
                FrameIt++;
                BoundingBoxIt++;
                OcclusionIt++;
                ++progressBarExtractROI;
                key = waitKey(50);
                switch((int)key){
                case 27 :   //QUIT
                    FrameIt = FramePath.end();break;
                case 105 : //INFOS
                    drawInfo=!drawInfo;break;
                case 32 : //LECTURE MODE
                    automode=!automode;break;
                case 100 : //DRAW MODE
                    drawBox=!drawBox;break;
                case 99 : //CHANGE COLOR
                    if(++colorIdx>colors.size()-1)
                        colorIdx=0;
                    break;
                case 109 : //CHANGE DELAY
                    if(drawDelay+1<drawDelayMax)
                        drawDelay++;
                    break;
                case 108 : //CHANGE DELAY
                    if(drawDelay-1>0)
                        drawDelay--;
                    break;
                }
            }
        }
    }while (FrameIt != FramePath.end());



    return 0;
}
