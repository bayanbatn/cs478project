#ifndef _PARAMSETREQUEST_H
#define _PARAMSETREQUEST_H

#include "Common.h"

#define HISTOGRAM_SIZE 256

/* [CS478]
 * These are allowed message types (for requests that are processed
 * by the work thread.)
 *
 * PARAM_PREVIEW_* relates to parameters for the viewfinder images.
 * PARAM_PREVIEW_AUTO_*_ON indicates that the request is asking for auto-* to be turned
 * on or off. These messages are generated as a result of user clicking the appropriate
 * checkboxes.
 *
 * PARAM_TAKE_PICTURE indicates that the request is asking for a shot to be taken.
 *
 * Others should be self-explanatory.
 *
 * See the switch-case statement in the main loop of the work thread, to see how
 * they are parsed.
 */
#define PARAM_SHOT                     0
#define PARAM_RESOLUTION               1
#define PARAM_BURST_SIZE               2
#define PARAM_OUTPUT_FORMAT            3
#define PARAM_VIEWER_ACTIVE            4
#define PARAM_OUTPUT_DIRECTORY         5
#define PARAM_OUTPUT_FILE_ID           6
#define PARAM_LUMINANCE_HISTOGRAM      7
#define PARAM_PREVIEW_EXPOSURE         8
#define PARAM_PREVIEW_FOCUS            9
#define PARAM_PREVIEW_GAIN             10
#define PARAM_PREVIEW_WB               11
#define PARAM_PREVIEW_AUTO_EXPOSURE_ON 12
#define PARAM_PREVIEW_AUTO_FOCUS_ON    13
#define PARAM_PREVIEW_AUTO_GAIN_ON     14
#define PARAM_PREVIEW_AUTO_WB_ON       15
#define PARAM_CAPTURE_FPS              16
#define PARAM_TAKE_PICTURE             17
/* [CS478]
 * The constants above are message types parsed by the work thread.
 * (Go look at the giant switch-case statement in FCamInterface.cpp.)
 * You can define new constants here, and add an appropriate handling
 * to the switch-case statement, in order to define new types of
 * messages, if needed.
 */
#define PARAM_AUTO_FOCUS_GLOBAL			18
#define PARAM_DEPTH_FOCUS_SWEEP			19
#define PARAM_IMAGE_BLUR				20

#define PARAM_PRIV_FS_CHANGED     100

#define SHOT_PARAM_EXPOSURE 0
#define SHOT_PARAM_FOCUS    1
#define SHOT_PARAM_GAIN     2
#define SHOT_PARAM_WB       3
#define SHOT_PARAM_FLASH    4

class ParamSetRequest {
public:
	ParamSetRequest(void) {
		m_id = -1;
		m_dataSize = 0;
		m_data = 0;
		m_aux_data = 0;
		m_aux_float = 0;
	}

	ParamSetRequest(int param, const void *str1, const void *str2, int data_size, float depth) {
		m_id = param;
		/* Init data */
		if (str1 != 0 && str2 != 0){
			m_data = (uchar*) str1;
			m_dataSize = data_size;
			m_aux_data = (uchar*) str2;
		}
		m_aux_float = depth;
	}

	ParamSetRequest(int param, const void *data, int dataSize) {
		m_id = param;
		m_dataSize = dataSize;
		m_data = new uchar[m_dataSize];
		if (data != 0) memcpy(m_data, data, m_dataSize);
		m_aux_data = 0;
		m_aux_float = 0;
	}

	ParamSetRequest(const ParamSetRequest &instance) {
		// copy constructor
		m_id = instance.m_id;
		m_dataSize = instance.m_dataSize;
		m_aux_float = instance.m_aux_float;
		m_data = new uchar[m_dataSize];
		memcpy(m_data, instance.m_data, m_dataSize);
		m_aux_data = 0;
		if (instance.m_aux_data != 0){
			int aux_data_len = strlen(((char*)instance.m_aux_data)) + 1;
			m_aux_data = new uchar[aux_data_len];
			memcpy(m_aux_data, instance.m_aux_data, aux_data_len);
		}
	}

	ParamSetRequest &operator=(const ParamSetRequest &instance) {
		m_id = instance.m_id;
		m_dataSize = instance.m_dataSize;
		m_aux_float = instance.m_aux_float;
		if (m_data != 0) {
			delete [] m_data;
		}
		if (m_aux_data != 0) {
			delete [] m_aux_data;
			m_aux_data = 0;
		}

		m_data = new uchar[m_dataSize];
		memcpy(m_data, instance.m_data, m_dataSize);
		if (instance.m_aux_data != 0){
			int aux_data_len = strlen(((char*)instance.m_aux_data)) + 1;
			m_aux_data = new uchar[aux_data_len];
			memcpy(m_aux_data, instance.m_aux_data, aux_data_len);
		}

		return *this;
	}

	~ParamSetRequest(void) {
		if (m_data != 0) {
			delete [] m_data;
		}
		if (m_aux_data != 0)
			delete[] m_aux_data;
	}

	int getId(void) {
		return m_id;
	}

	uchar *getData(void) {
		return m_data;
	}

	int getDataAsInt(void) {
		return ((int *) m_data)[0];
	}

	int getDataSize(void) {
		return m_dataSize;
	}

	uchar *getAuxData(void){
		return m_aux_data;
	}

	float getAuxFloat(void){
		return m_aux_float;
	}

private:
	int m_id;
	uchar *m_data; // serialized param value
	int m_dataSize;
	uchar *m_aux_data;
	float m_aux_float;
};


#endif


