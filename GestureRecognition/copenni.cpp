#include "copenni.h"


using namespace xn;
using namespace std;

COpenNI::COpenNI()
{
}

COpenNI::~COpenNI()
{
}

bool COpenNI::Initial()
{
	status_ = context_.Init();
	if(CheckError("Context initial failed!")) 
	{
		return false;
	}

	context_.SetGlobalMirror(true);//���þ���
	xmode_.nXRes = 640;
	xmode_.nYRes = 480;
	xmode_.nFPS = 30;

	//������ɫnode
	status_ = image_generator_.Create(context_);
	if(CheckError("Create image generator  error!"))
	{
		return false;
	}

	//������ɫͼƬ���ģʽ
	status_ = image_generator_.SetMapOutputMode(xmode_);
	if(CheckError("SetMapOutputMdoe error!")) {
		return false;
	}

	//�������node
	status_ = depth_generator_.Create(context_);
	if(CheckError("Create depth generator  error!"))
	{
		return false;
	}

	//�������ͼƬ���ģʽ
	status_ = depth_generator_.SetMapOutputMode(xmode_);
	if(CheckError("SetMapOutputMdoe error!")) 
	{
		return false;
	}

	//��������node
	status_ = gesture_generator_.Create(context_);
	if(CheckError("Create gesture generator error!"))
	{
		return false;
	}

	/*�������ʶ�������*/
	gesture_generator_.AddGesture("Wave", NULL);
	gesture_generator_.AddGesture("click", NULL);
	gesture_generator_.AddGesture("RaiseHand", NULL);
	gesture_generator_.AddGesture("MovingHand", NULL);

	//�����ֲ���node
	status_ = hand_generator_.Create(context_);
	if(CheckError("Create hand generaotr error!"))
	{
		return false;
	}

	//��������node
	status_ = user_generator_.Create(context_);
	if(CheckError("Create gesture generator error!")) 
	{
		return false;
	}

	//�ӽ�У��
	status_ = depth_generator_.GetAlternativeViewPointCap().SetViewPoint(image_generator_);
	if(CheckError("Can't set the alternative view point on depth generator!")) 
	{
		return false;
	}

	//�����������йصĻص�����
	XnCallbackHandle gesture_cb;
	gesture_generator_.RegisterGestureCallbacks(CBGestureRecognized, CBGestureProgress, this, gesture_cb);

	//�������ֲ��йصĻص�����
	XnCallbackHandle hands_cb;
	hand_generator_.RegisterHandCallbacks(HandCreate, HandUpdate, HandDestroy, this, hands_cb);

	//�������˽�����Ұ�Ļص�����
	XnCallbackHandle new_user_handle;
	user_generator_.RegisterUserCallbacks(CBNewUser, NULL, NULL, new_user_handle);
	user_generator_.GetSkeletonCap().SetSkeletonProfile(XN_SKEL_PROFILE_ALL);//�趨ʹ�����йؽڣ���15����

	//���ù���У����ɵĻص�����
	XnCallbackHandle calibration_complete;
	user_generator_.GetSkeletonCap().RegisterToCalibrationComplete(CBCalibrationComplete, this, calibration_complete);
	return true;
}

bool COpenNI::Start()
{
	status_ = context_.StartGeneratingAll();
	if(CheckError("Start generating error!"))
	{
		return false;
	}
	return true;
}

bool COpenNI::UpdateData()
{
	status_ = context_.WaitNoneUpdateAll();
	if(CheckError("Update date error!")) 
	{
		return false;
	}
	//��ȡ����
	image_generator_.GetMetaData(image_metadata_);
	depth_generator_.GetMetaData(depth_metadata_);

	return true;
}

ImageGenerator &COpenNI::getImageGenerator()
{
	return image_generator_;
}

DepthGenerator &COpenNI::getDepthGenerator()
{
	return depth_generator_;
}

UserGenerator &COpenNI::getUserGenerator()
{
	return user_generator_;
}

GestureGenerator &COpenNI::getGestureGenerator()
{
	return gesture_generator_;
}

HandsGenerator &COpenNI::getHandGenerator()
{
	return hand_generator_;
}

bool COpenNI::CheckError(const char *error)
{
	if(status_ != XN_STATUS_OK) 
	{
		cerr << error << ": " << xnGetStatusString( status_ ) << endl;
		return true;
	}
	return false;
}

void COpenNI::CBNewUser(UserGenerator &generator, XnUserID user, void *p_cookie)
{
	//�õ�skeleton��capability,������RequestCalibration�������ö��¼�⵽���˽��й���У��
	generator.GetSkeletonCap().RequestCalibration(user, true);
}

void COpenNI::CBCalibrationComplete(SkeletonCapability &skeleton, XnUserID user, XnCalibrationStatus calibration_error, void *p_cookie)
{
	if(calibration_error == XN_CALIBRATION_STATUS_OK) 
	{
		skeleton.StartTracking(user);//����У����ɺ�Ϳ�ʼ�������������
	}
	else 
	{
		UserGenerator *p_user = (UserGenerator*)p_cookie;
		skeleton.RequestCalibration(user, true);//����У��ʧ��ʱ�������ö����������������У��
	}
}

void COpenNI::CBGestureRecognized(GestureGenerator &generator, const XnChar *strGesture, const XnPoint3D *pIDPosition, const XnPoint3D *pEndPosition, void *pCookie)
{
	COpenNI *openni = (COpenNI*)pCookie;
	openni->hand_generator_.StartTracking(*pEndPosition);
}

void COpenNI::CBGestureProgress(GestureGenerator &generator, const XnChar *strGesture, const XnPoint3D *pPosition, XnFloat fProgress, void *pCookie)
{
}

void COpenNI::HandCreate(HandsGenerator &rHands, XnUserID xUID, const XnPoint3D *pPosition, XnFloat fTime, void *pCookie)
{
	COpenNI *openni = (COpenNI*)pCookie;
	XnPoint3D project_pos;
	openni->depth_generator_.ConvertRealWorldToProjective(1, pPosition, &project_pos);
	pair<XnUserID, XnPoint3D> hand_point_pair(xUID, XnPoint3D());//�ڽ���pair���͵Ķ���ʱ�����Խ���2������Ϊ��
	hand_point_pair.second = project_pos;
	openni->hand_points_.insert(hand_point_pair);//����⵽���ֲ�����map���͵�hand_points_�С�

	pair<XnUserID, vector<XnPoint3D>> hand_track_point(xUID, vector<XnPoint3D>());
	hand_track_point.second.push_back(project_pos);
	openni->hands_track_points_.insert(hand_track_point);
}

void COpenNI::HandUpdate(HandsGenerator &rHands, XnUserID xUID, const XnPoint3D *pPosition, XnFloat fTime, void *pCookie)
{
	COpenNI *openni = (COpenNI*)pCookie;
	XnPoint3D project_pos;
	openni->depth_generator_.ConvertRealWorldToProjective(1, pPosition, &project_pos);
	openni->hand_points_.find(xUID)->second = project_pos;
	openni->hands_track_points_.find(xUID)->second.push_back(project_pos);
}

void COpenNI::HandDestroy(HandsGenerator &rHands, XnUserID xUID, XnFloat fTime, void *pCookie)
{
	COpenNI *openni = (COpenNI*)pCookie;
	openni->hand_points_.erase(openni->hand_points_.find(xUID));
	openni->hands_track_points_.erase(openni->hands_track_points_.find(xUID ));
}

void COpenNI::Stop()
{
	context_.StopGeneratingAll();
}
