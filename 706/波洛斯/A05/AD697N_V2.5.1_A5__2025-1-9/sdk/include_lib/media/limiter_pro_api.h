#ifndef LIMITER_PRO_API
#define LIMITER_PRO_API

#include "AudioEffect_DataType.h"

#ifdef WIN32
#define AT_LIMITER_PRO(x)
#define AT_LIMITER_PRO_CODE
#define AT_LIMITER_PRO_CONST
#define AT_LIMITER_PRO_SPARSE_CODE
#define AT_LIMITER_PRO_SPARSE_CONST
#else
#define AT_LIMITER_PRO(x)            __attribute__((section(#x)))
#define AT_LIMITER_PRO_CODE			 AT_LIMITER_PRO(.limiter_pro.text.cache.L2.code)
#define AT_LIMITER_PRO_CONST		 AT_LIMITER_PRO(.limiter_pro.text.cache.L2.const)
#define AT_LIMITER_PRO_SPARSE_CODE	 AT_LIMITER_PRO(.limiter_pro.text)
#define AT_LIMITER_PRO_SPARSE_CONST	 AT_LIMITER_PRO(.limiter_pro.text.const)
#endif

struct limiter_param {
    int channel;
    int sample_rate;
    int attack_time;
    int release_time;
    int threshold;
    int hold_time;
    int detect_time;
    int input_gain;
    int output_gain;
    int precision;
    int resever[6];
    af_DataType pcm_info;
};

int get_limiter_pro_buf(struct limiter_param *param);
int limiter_pro_init(void *work_buf, struct limiter_param *param);
int limiter_pro_update(void *work_buf, struct limiter_param *param);
int limiter_pro_run(void *work_buf, void *indata, void *outdata, int per_channel_npoint);

#endif // !LIMITER_PRO_API
