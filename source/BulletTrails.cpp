#include "plugin.h"
#include "Utility.h"
#include "BulletTrails.h"
#include "CCamera.h"
#include "CTxdStore.h"
#include "RenderBuffer.h"
#include "ini.h"
#include <format>
#define ARRAY_SIZE(array)                (sizeof(array) / sizeof(array[0]))
#define FIX_BUGS // Undefine to play with bugs
RwTexture* gpSmokeTrailTexture;
RwTexture* gpTraceTexture;

float CBulletTraces::thickness[512];
int CBulletTraces::lifetime[512];
int CBulletTraces::visibility[512];
int CBulletTraces::type[512];

RwIm3DVertex VCTraceVertices[10];
static RwImVertexIndex VCTraceIndexList[48] = { 0, 5, 7, 0, 7, 2, 0, 7, 5, 0, 2, 7, 0, 4, 9, 0,
										9, 5, 0, 9, 4, 0, 5, 9, 0, 1, 6, 0, 6, 5, 0, 6,
										1, 0, 5, 6, 0, 3, 8, 0, 8, 5, 0, 8, 3, 0, 5, 8 };

RwIm3DVertex TraceVerticesIII[6];
RwImVertexIndex TraceIndexListIII[12];


void CBulletTraces::Init(void)
{
	for (int i = 0; i < 255; i++)
		aTraces[i].m_bInUse = false;
	CTxdStore::PushCurrentTxd();
	int32_t slot2 = CTxdStore::AddTxdSlot("VCBulletTrails");
	CTxdStore::LoadTxd(slot2, GAME_PATH((char*)"MODELS\\VCBULLETTRAILS.TXD"));
	int32_t slot = CTxdStore::FindTxdSlot("VCBulletTrails");
	CTxdStore::SetCurrentTxd(slot);
	gpSmokeTrailTexture = RwTextureRead("smoketrail", NULL);
	gpTraceTexture = RwTextureRead("3trace", NULL);
	CTxdStore::PopCurrentTxd();

	// III

	RwIm3DVertexSetRGBA(&TraceVerticesIII[0], 20, 20, 20, 255);
	RwIm3DVertexSetRGBA(&TraceVerticesIII[1], 20, 20, 20, 255);
	RwIm3DVertexSetRGBA(&TraceVerticesIII[2], 70, 70, 70, 255);
	RwIm3DVertexSetRGBA(&TraceVerticesIII[3], 70, 70, 70, 255);
	RwIm3DVertexSetRGBA(&TraceVerticesIII[4], 10, 10, 10, 255);
	RwIm3DVertexSetRGBA(&TraceVerticesIII[5], 10, 10, 10, 255);
	RwIm3DVertexSetU(&TraceVerticesIII[0], 0.0);
	RwIm3DVertexSetV(&TraceVerticesIII[0], 0.0);
	RwIm3DVertexSetU(&TraceVerticesIII[1], 1.0);
	RwIm3DVertexSetV(&TraceVerticesIII[1], 0.0);
	RwIm3DVertexSetU(&TraceVerticesIII[2], 0.0);
	RwIm3DVertexSetV(&TraceVerticesIII[2], 0.5);
	RwIm3DVertexSetU(&TraceVerticesIII[3], 1.0);
	RwIm3DVertexSetV(&TraceVerticesIII[3], 0.5);
	RwIm3DVertexSetU(&TraceVerticesIII[4], 0.0);
	RwIm3DVertexSetV(&TraceVerticesIII[4], 1.0);
	RwIm3DVertexSetU(&TraceVerticesIII[5], 1.0);
	RwIm3DVertexSetV(&TraceVerticesIII[5], 1.0);

	TraceIndexListIII[0] = 0;
	TraceIndexListIII[1] = 2;
	TraceIndexListIII[2] = 1;
	TraceIndexListIII[3] = 1;
	TraceIndexListIII[4] = 2;
	TraceIndexListIII[5] = 3;
	TraceIndexListIII[6] = 2;
	TraceIndexListIII[7] = 4;
	TraceIndexListIII[8] = 3;
	TraceIndexListIII[9] = 3;
	TraceIndexListIII[10] = 4;
	TraceIndexListIII[11] = 5;

	// VC
	RwIm3DVertexSetU(&VCTraceVertices[0], 0.0);
	RwIm3DVertexSetV(&VCTraceVertices[0], 0.0);
	RwIm3DVertexSetU(&VCTraceVertices[1], 1.0);
	RwIm3DVertexSetV(&VCTraceVertices[1], 0.0);
	RwIm3DVertexSetU(&VCTraceVertices[2], 1.0);
	RwIm3DVertexSetV(&VCTraceVertices[2], 0.0);
	RwIm3DVertexSetU(&VCTraceVertices[3], 1.0);
	RwIm3DVertexSetV(&VCTraceVertices[3], 0.0);
	RwIm3DVertexSetU(&VCTraceVertices[4], 1.0);
	RwIm3DVertexSetV(&VCTraceVertices[4], 0.0);
	RwIm3DVertexSetU(&VCTraceVertices[5], 0.0);
	RwIm3DVertexSetU(&VCTraceVertices[6], 1.0);
	RwIm3DVertexSetU(&VCTraceVertices[7], 1.0);
	RwIm3DVertexSetU(&VCTraceVertices[8], 1.0);
	RwIm3DVertexSetU(&VCTraceVertices[9], 1.0);

	VCTraceIndexList[0] = 0;
	VCTraceIndexList[1] = 2;
	VCTraceIndexList[2] = 1;
	VCTraceIndexList[3] = 1;
	VCTraceIndexList[4] = 2;
	VCTraceIndexList[5] = 3;
	VCTraceIndexList[6] = 2;
	VCTraceIndexList[7] = 4;
	VCTraceIndexList[8] = 3;
	VCTraceIndexList[9] = 3;
	VCTraceIndexList[10] = 4;
	VCTraceIndexList[11] = 5;

	//Traces ini load
	mINI::INIFile file(GAME_PATH((char*)"MODELS\\VCBulletTrails.ini"));
	mINI::INIStructure ini;
	file.read(ini);

	//Reading weapons params
	for (int32_t i = 22; i < 512; i++)
	{

		std::string name = "WEP";
		std::string formatted_str = std::format(
			"{}{}", name,
			i);

		const char* formatted_str2 = formatted_str.c_str();

		std::string strb = ini.get(formatted_str2).get("thickness");
		const char* strb2 = strb.c_str();
		std::string strc = ini.get(formatted_str2).get("lifetime");
		const char* strc2 = strc.c_str();
		std::string strd = ini.get(formatted_str2).get("visibility");
		const char* strd2 = strd.c_str();
		std::string stre = ini.get(formatted_str2).get("type");
		const char* stre2 = stre.c_str();

		thickness[i] = std::atof(strb2);
		lifetime[i] = std::atoi(strc2);
		visibility[i] = std::atoi(strd2);
		type[i] = std::atoi(stre2);

	}

}

CBulletTrace CBulletTraces::aTraces[255];

void CBulletTraces::AddTrace(CVector* start, CVector* end, float thickness, uint32_t lifeTime, uint8_t visibility, uint8_t type)
{
	int32_t enabledCount;
	uint32_t modifiedLifeTime;
	int32_t nextSlot;

	enabledCount = 0;
	for (int i = 0; i < 255; i++)
		if (aTraces[i].m_bInUse)
			enabledCount++;

	switch (type) {
		case TYPE_III:
			modifiedLifeTime = 25 + GetRandomNumber() % 32;
			break;
		case TYPE_VC:
			if (enabledCount >= 10)
				modifiedLifeTime = lifeTime / 4;
			else if (enabledCount >= 5)
				modifiedLifeTime = lifeTime / 2;
			else
				modifiedLifeTime = lifeTime;
			break;
		default:
			break;
	}

	nextSlot = 0;
	for (int i = 0; nextSlot < 255 && aTraces[i].m_bInUse; i++)
		nextSlot++;
	if (nextSlot < 255) {
		aTraces[nextSlot].m_vecStartPos = *start;
		aTraces[nextSlot].m_vecEndPos = *end;
		aTraces[nextSlot].m_bInUse = true;
		aTraces[nextSlot].m_nCreationTime = CTimer::m_snTimeInMilliseconds;
		aTraces[nextSlot].m_fVisibility = visibility;
		aTraces[nextSlot].m_fThickness = thickness;
		aTraces[nextSlot].type = type;
		aTraces[nextSlot].m_framesInUse = 0;
		aTraces[nextSlot].m_nLifeTime = modifiedLifeTime;
	}

	float startProjFwd = DotProduct(TheCamera.GetForward(), *start - TheCamera.GetPosition());
	float endProjFwd = DotProduct(TheCamera.GetForward(), *end - TheCamera.GetPosition());
	if (startProjFwd * endProjFwd < 0.0f) { //if one of point behind us and second before us
		float fStartDistFwd = abs(startProjFwd) / (abs(startProjFwd) + abs(endProjFwd));

		float startProjUp = DotProduct(TheCamera.GetUp(), *start - TheCamera.GetPosition());
		float endProjUp = DotProduct(TheCamera.GetUp(), *end - TheCamera.GetPosition());
		float distUp = (endProjUp - startProjUp) * fStartDistFwd + startProjUp;

		float startProjRight = DotProduct(TheCamera.GetRight(), *start - TheCamera.GetPosition());
		float endProjRight = DotProduct(TheCamera.GetRight(), *end - TheCamera.GetPosition());
		float distRight = (endProjRight - startProjRight) * fStartDistFwd + startProjRight;

		float dist = sqrt(SQR(distUp) + SQR(distRight));
	}
}

void CBulletTraces::AddTrace2(CVector* start, CVector* end, int32_t weaponType, class CEntity* shooter)
{
	CPhysical* player;
	float speed;
	int16_t camMode;

	if (shooter == (CEntity*)FindPlayerPed() || (FindPlayerVehicle(NULL, NULL) != NULL && FindPlayerVehicle(NULL, NULL) == (CVehicle*)shooter)) {
		camMode = TheCamera.m_aCams[TheCamera.m_nActiveCam].m_nMode;
		if (camMode == MODE_M16_1STPERSON
			|| camMode == MODE_CAMERA
			|| camMode == MODE_SNIPER
			|| camMode == MODE_M16_1STPERSON_RUNABOUT
			|| camMode == MODE_ROCKETLAUNCHER
			|| camMode == MODE_ROCKETLAUNCHER_RUNABOUT
			|| camMode == MODE_SNIPER_RUNABOUT
			|| camMode == MODE_HELICANNON_1STPERSON) {

			player = FindPlayerVehicle(NULL, NULL) ? (CPhysical*)FindPlayerVehicle(NULL, NULL) : (CPhysical*)FindPlayerPed();
			speed = player->m_vecMoveSpeed.Magnitude();
			if (speed < 0.05f)
				return;
		}
	}

	/*AddTrace(
		start,
		end,
		1.0f,
		2000,
		220
	);
	*/

	AddTrace(
		start,
		end,
		thickness[weaponType],
		lifetime[weaponType],
		visibility[weaponType],
		type[weaponType]
	);
}

CVector operator/(const CVector& vec, float dividend) {
	return { vec.x / dividend, vec.y / dividend, vec.z / dividend };
}

void CBulletTraces::Render(void)
{
	for (int i = 0; i < 255; i++) {
		if (!aTraces[i].m_bInUse)
			continue;

		switch (aTraces[i].type) {
			case TYPE_III:
				Render_III(i);
				break;
			case TYPE_VC:
				Render_VC(i);
				break;
			default:
				break;
		}
	}
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
}

void CBulletTraces::Update(void)
{
	for (int i = 0; i < 255; i++) {
		if (aTraces[i].m_bInUse)
			aTraces[i].Update();
	}
}

void CBulletTrace::Update(void)
{
	CVector diff;
	float remaining;

	switch (type) {
	case TYPE_III:
		if (m_framesInUse == 0) {
			m_framesInUse++;
			return;
		}
		if (m_framesInUse > 60) {
			m_bInUse = false;
			return;
		}
		diff = m_vecStartPos - m_vecEndPos;
		remaining = diff.Magnitude();
		if (remaining > 0.8f)
			m_vecStartPos = m_vecEndPos + (remaining - 0.8f) / remaining * diff;
		else
			m_bInUse = false;
		if (--m_nLifeTime == 0)
			m_bInUse = false;
		m_framesInUse++;
		break;
	case TYPE_VC:
		if (CTimer::m_snTimeInMilliseconds - m_nCreationTime >= m_nLifeTime)
			m_bInUse = false;
		break;
	default:
		break;
	}
}

void CBulletTraces::Shutdown(void) {
	if (gpSmokeTrailTexture) {
		RwTextureDestroy(gpSmokeTrailTexture);
		gpSmokeTrailTexture = nullptr;
	}

	if (gpTraceTexture) {
		RwTextureDestroy(gpTraceTexture);
		gpTraceTexture = nullptr;
	}
}

void CBulletTraces::Render_III(int current_slot) {
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)FALSE);
#ifdef FIX_BUGS
	// Raster has no transparent pixels so it relies on the raster format having alpha
	// to turn on blending. librw image conversion might get rid of it right now so let's
	// just force it on.
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
#endif
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDONE);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDONE);
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, RwTextureGetRaster(gpTraceTexture));
	CVector inf = aTraces[current_slot].m_vecStartPos;
	CVector sup = aTraces[current_slot].m_vecEndPos;
	CVector center = (inf + sup) / 2;
	CVector width = aTraces[current_slot].CrossProduct(TheCamera.GetForward(), (sup - inf));
	width.Normalise();
	width /= 20;
	uint8_t intensity = aTraces[current_slot].m_nLifeTime;
	for (int i = 0; i < ARRAY_SIZE(TraceVerticesIII); i++)
		RwIm3DVertexSetRGBA(&TraceVerticesIII[i], intensity, intensity, intensity, 0xFF);
	RwIm3DVertexSetPos(&TraceVerticesIII[0], inf.x + width.x, inf.y + width.y, inf.z + width.z);
	RwIm3DVertexSetPos(&TraceVerticesIII[1], inf.x - width.x, inf.y - width.y, inf.z - width.z);
	RwIm3DVertexSetPos(&TraceVerticesIII[2], center.x + width.x, center.y + width.y, center.z + width.z);
	RwIm3DVertexSetPos(&TraceVerticesIII[3], center.x - width.x, center.y - width.y, center.z - width.z);
	RwIm3DVertexSetPos(&TraceVerticesIII[4], sup.x + width.x, sup.y + width.y, sup.z + width.z);
	RwIm3DVertexSetPos(&TraceVerticesIII[5], sup.x - width.x, sup.y - width.y, sup.z - width.z);
	if (RwIm3DTransform(TraceVerticesIII, ARRAY_SIZE(TraceVerticesIII), NULL, rwIM3D_VERTEXUV)) {
		RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST, TraceIndexListIII, ARRAY_SIZE(TraceIndexListIII));
		RwIm3DEnd();
	}
}

void CBulletTraces::Render_VC(int current_slot) {
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void*)(RwTextureGetRaster(gpSmokeTrailTexture)));

	float timeAlive = CTimer::m_snTimeInMilliseconds - aTraces[current_slot].m_nCreationTime;

	float traceThickness = aTraces[current_slot].m_fThickness * timeAlive / aTraces[current_slot].m_nLifeTime;

	CVector horizontalOffset = aTraces[current_slot].m_vecEndPos - aTraces[current_slot].m_vecStartPos;
	horizontalOffset.Normalise();
	horizontalOffset *= traceThickness;

	//then closer trace to die then it more transparent
	uint8_t nAlphaValue = aTraces[current_slot].m_fVisibility * (aTraces[current_slot].m_nLifeTime - timeAlive) / aTraces[current_slot].m_nLifeTime;

	CVector start = aTraces[current_slot].m_vecStartPos;
	CVector end = aTraces[current_slot].m_vecEndPos;
	float startProj = DotProduct(start - TheCamera.GetPosition(), TheCamera.GetForward()) - 0.7f;
	float endProj = DotProduct(end - TheCamera.GetPosition(), TheCamera.GetForward()) - 0.7f;
	if (startProj < 0.0f && endProj < 0.0f) //we dont need render trace behind us
		return;

	if (startProj < 0.0f) { //if strat behind us move it closer
		float absStartProj = std::abs(startProj);
		float absEndProj = std::abs(endProj);
		start = (absEndProj * start + absStartProj * end) / (absStartProj + absEndProj);
	}
	else if (endProj < 0.0f) {
		float absStartProj = std::abs(startProj);
		float absEndProj = std::abs(endProj);
		end = (absEndProj * start + absStartProj * end) / (absStartProj + absEndProj);
	}

	//we divide trace at three parts
	CVector start2 = (7.0f * start + end) / 8;
	CVector end2 = (7.0f * end + start) / 8;

	RwIm3DVertexSetV(&VCTraceVertices[5], 10.0f);
	RwIm3DVertexSetV(&VCTraceVertices[6], 10.0f);
	RwIm3DVertexSetV(&VCTraceVertices[7], 10.0f);
	RwIm3DVertexSetV(&VCTraceVertices[8], 10.0f);
	RwIm3DVertexSetV(&VCTraceVertices[9], 10.0f);

	RwIm3DVertexSetRGBA(&VCTraceVertices[0], 255, 255, 255, nAlphaValue);
	RwIm3DVertexSetRGBA(&VCTraceVertices[1], 255, 255, 255, nAlphaValue);
	RwIm3DVertexSetRGBA(&VCTraceVertices[2], 255, 255, 255, nAlphaValue);
	RwIm3DVertexSetRGBA(&VCTraceVertices[3], 255, 255, 255, nAlphaValue);
	RwIm3DVertexSetRGBA(&VCTraceVertices[4], 255, 255, 255, nAlphaValue);
	RwIm3DVertexSetRGBA(&VCTraceVertices[5], 255, 255, 255, nAlphaValue);
	RwIm3DVertexSetRGBA(&VCTraceVertices[6], 255, 255, 255, nAlphaValue);
	RwIm3DVertexSetRGBA(&VCTraceVertices[7], 255, 255, 255, nAlphaValue);
	RwIm3DVertexSetRGBA(&VCTraceVertices[8], 255, 255, 255, nAlphaValue);
	RwIm3DVertexSetRGBA(&VCTraceVertices[9], 255, 255, 255, nAlphaValue);
	//two points in center
	RwIm3DVertexSetPos(&VCTraceVertices[0], start2.x, start2.y, start2.z);
	RwIm3DVertexSetPos(&VCTraceVertices[5], end2.x, end2.y, end2.z);
	//vertical planes
	RwIm3DVertexSetPos(&VCTraceVertices[1], start2.x, start2.y, start2.z + traceThickness);
	RwIm3DVertexSetPos(&VCTraceVertices[3], start2.x, start2.y, start2.z - traceThickness);
	RwIm3DVertexSetPos(&VCTraceVertices[6], end2.x, end2.y, end2.z + traceThickness);
	RwIm3DVertexSetPos(&VCTraceVertices[8], end2.x, end2.y, end2.z - traceThickness);
	//horizontal planes
	RwIm3DVertexSetPos(&VCTraceVertices[2], start2.x + horizontalOffset.y, start2.y - horizontalOffset.x, start2.z);
	RwIm3DVertexSetPos(&VCTraceVertices[7], end2.x + horizontalOffset.y, end2.y - horizontalOffset.x, end2.z);
#ifdef FIX_BUGS //this point calculated wrong for some reason
	RwIm3DVertexSetPos(&VCTraceVertices[4], start2.x - horizontalOffset.y, start2.y + horizontalOffset.x, start2.z);
	RwIm3DVertexSetPos(&VCTraceVertices[9], end2.x - horizontalOffset.y, end2.y + horizontalOffset.x, end2.z);
#else
	RwIm3DVertexSetPos(&VCTraceVertices[4], start2.x - horizontalOffset.y, start2.y - horizontalOffset.y, start2.z);
	RwIm3DVertexSetPos(&VCTraceVertices[9], end2.x - horizontalOffset.y, end2.y - horizontalOffset.y, end2.z);
#endif

	if (RwIm3DTransform(VCTraceVertices, ARRAY_SIZE(VCTraceVertices), nullptr, 1)) {
		RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST, VCTraceIndexList, ARRAY_SIZE(VCTraceIndexList));
		RwIm3DEnd();
	}

	RwIm3DVertexSetV(&VCTraceVertices[5], 2.0f);
	RwIm3DVertexSetV(&VCTraceVertices[6], 2.0f);
	RwIm3DVertexSetV(&VCTraceVertices[7], 2.0f);
	RwIm3DVertexSetV(&VCTraceVertices[8], 2.0f);
	RwIm3DVertexSetV(&VCTraceVertices[9], 2.0f);
	RwIm3DVertexSetRGBA(&VCTraceVertices[0], 255, 255, 255, 0);
	RwIm3DVertexSetRGBA(&VCTraceVertices[1], 255, 255, 255, 0);
	RwIm3DVertexSetRGBA(&VCTraceVertices[2], 255, 255, 255, 0);
	RwIm3DVertexSetRGBA(&VCTraceVertices[3], 255, 255, 255, 0);
	RwIm3DVertexSetRGBA(&VCTraceVertices[4], 255, 255, 255, 0);

	RwIm3DVertexSetPos(&VCTraceVertices[0], start.x, start.y, start.z);
	RwIm3DVertexSetPos(&VCTraceVertices[1], start.x, start.y, start.z + traceThickness);
	RwIm3DVertexSetPos(&VCTraceVertices[3], start.x, start.y, start.z - traceThickness);
	RwIm3DVertexSetPos(&VCTraceVertices[2], start.x + horizontalOffset.y, start.y - horizontalOffset.x, start.z);

	RwIm3DVertexSetPos(&VCTraceVertices[5], start2.x, start2.y, start2.z);
	RwIm3DVertexSetPos(&VCTraceVertices[6], start2.x, start2.y, start2.z + traceThickness);
	RwIm3DVertexSetPos(&VCTraceVertices[8], start2.x, start2.y, start2.z - traceThickness);
	RwIm3DVertexSetPos(&VCTraceVertices[7], start2.x + horizontalOffset.y, start2.y - horizontalOffset.x, start2.z);
#ifdef FIX_BUGS
	RwIm3DVertexSetPos(&VCTraceVertices[4], start.x - horizontalOffset.y, start.y + horizontalOffset.x, start.z);
	RwIm3DVertexSetPos(&VCTraceVertices[9], start2.x - horizontalOffset.y, start2.y + horizontalOffset.x, start2.z);
#else
	RwIm3DVertexSetPos(&VCTraceVertices[4], start.x - horizontalOffset.y, start.y - horizontalOffset.y, start.z);
	RwIm3DVertexSetPos(&VCTraceVertices[9], start2.x - horizontalOffset.y, start2.y - horizontalOffset.y, start2.z);
#endif

	if (RwIm3DTransform(VCTraceVertices, ARRAY_SIZE(VCTraceVertices), nullptr, rwIM3D_VERTEXUV)) {
		RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST, VCTraceIndexList, ARRAY_SIZE(VCTraceIndexList));
		RwIm3DEnd();
	}

	RwIm3DVertexSetPos(&VCTraceVertices[1], end.x, end.y, end.z);
	RwIm3DVertexSetPos(&VCTraceVertices[2], end.x, end.y, end.z + traceThickness);
	RwIm3DVertexSetPos(&VCTraceVertices[4], end.x, end.y, end.z - traceThickness);
	RwIm3DVertexSetPos(&VCTraceVertices[3], end.x + horizontalOffset.y, end.y - horizontalOffset.x, end.z);

	RwIm3DVertexSetPos(&VCTraceVertices[5], end2.x, end2.y, end2.z);
	RwIm3DVertexSetPos(&VCTraceVertices[6], end2.x, end2.y, end2.z + traceThickness);
	RwIm3DVertexSetPos(&VCTraceVertices[8], end2.x, end2.y, end2.z - traceThickness);
	RwIm3DVertexSetPos(&VCTraceVertices[7], end2.x + horizontalOffset.y, end2.y - horizontalOffset.x, end2.z);
#ifdef FIX_BUGS
	RwIm3DVertexSetPos(&VCTraceVertices[5], end.x - horizontalOffset.y, end.y + horizontalOffset.x, end.z);
	RwIm3DVertexSetPos(&VCTraceVertices[9], end2.x - horizontalOffset.y, end2.y + horizontalOffset.x, end2.z);
#else
	RwIm3DVertexSetPos(&VCTraceVertices[5], end.x - horizontalOffset.y, end.y - horizontalOffset.y, end.z);
	RwIm3DVertexSetPos(&VCTraceVertices[9], end2.x - horizontalOffset.y, end2.y - horizontalOffset.y, end2.z);
#endif

	if (RwIm3DTransform(VCTraceVertices, ARRAY_SIZE(VCTraceVertices), nullptr, rwIM3D_VERTEXUV)) {
		RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST, VCTraceIndexList, ARRAY_SIZE(VCTraceIndexList));
		RwIm3DEnd();
	}
}