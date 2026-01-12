/* Simscape target specific file.
 * This file is generated for the Simscape network associated with the solver block 'motorLF1/motorLF1/Solver Configuration'.
 */

#include "ne_ds.h"
#include "motorLF1_446c021e_49_ds_sys_struct.h"
#include "motorLF1_446c021e_49_ds_log.h"
#include "motorLF1_446c021e_49_ds.h"
#include "motorLF1_446c021e_49_ds_externals.h"
#include "motorLF1_446c021e_49_ds_external_struct.h"
#include "ssc_ml_fun.h"

int32_T motorLF1_446c021e_49_ds_log(const NeDynamicSystem *sys, const
  NeDynamicSystemInput *t3, NeDsMethodOutput *out)
{
  real_T motorLF1_Controlled_Voltage_Source_i;
  real_T motorLF1_Ideal_Rotational_Motion_Sensor_W;
  real_T motorLF1_Mass_f;
  real_T motorLF1_Mass_v;
  real_T motorLF1_Simple_Gear_tB;
  real_T motorLF1_Tire_Simple_inertia_t;
  motorLF1_Controlled_Voltage_Source_i = (t3->mX.mX[0UL] * -0.99999999952338436
    + t3->mX.mX[1UL] * 4.7487132283774637E-11) + t3->mU.mX[0UL] *
    -9.9999999952338438E-10;
  motorLF1_Mass_f = (t3->mX.mX[0UL] * 8.6298642016208227 + t3->mX.mX[1UL] *
                     -0.10522200756069462) + t3->mU.mX[0UL] *
    8.6298642016208239E-9;
  motorLF1_Tire_Simple_inertia_t = (t3->mX.mX[0UL] * 0.0726346903636419 +
    t3->mX.mX[1UL] * -0.00088561856363584624) + t3->mU.mX[0UL] *
    7.2634690363641915E-11;
  out->mLOG.mX[13UL] = motorLF1_Mass_f * -0.0023255813953488372 +
    motorLF1_Tire_Simple_inertia_t * -0.023255813953488372;
  motorLF1_Mass_v = t3->mX.mX[1UL] * 0.0023255813953488372;
  motorLF1_Ideal_Rotational_Motion_Sensor_W = motorLF1_Mass_v * 10.0;
  out->mLOG.mX[32UL] = motorLF1_Mass_v * 430.0;
  motorLF1_Simple_Gear_tB = motorLF1_Mass_f * 0.0023255813953488372 +
    motorLF1_Tire_Simple_inertia_t * 0.023255813953488372;
  out->mLOG.mX[45UL] = t3->mX.mX[1UL] * 0.023255813953488372;
  out->mLOG.mX[0UL] = motorLF1_Controlled_Voltage_Source_i;
  out->mLOG.mX[1UL] = t3->mU.mX[0UL];
  out->mLOG.mX[2UL] = t3->mU.mX[0UL];
  out->mLOG.mX[3UL] = t3->mU.mX[0UL];
  out->mLOG.mX[4UL] = -motorLF1_Controlled_Voltage_Source_i;
  out->mLOG.mX[5UL] = -motorLF1_Controlled_Voltage_Source_i;
  out->mLOG.mX[6UL] = t3->mU.mX[0UL];
  out->mLOG.mX[7UL] = t3->mU.mX[0UL];
  out->mLOG.mX[8UL] = -motorLF1_Controlled_Voltage_Source_i;
  out->mLOG.mX[9UL] = motorLF1_Mass_v * 430.0;
  out->mLOG.mX[10UL] = -motorLF1_Controlled_Voltage_Source_i;
  out->mLOG.mX[11UL] = t3->mU.mX[0UL];
  out->mLOG.mX[12UL] = motorLF1_Controlled_Voltage_Source_i *
    motorLF1_Controlled_Voltage_Source_i * 476.61557223563062 * 1.0E-6 * 1000.0;
  out->mLOG.mX[14UL] = t3->mU.mX[0UL];
  out->mLOG.mX[15UL] = t3->mX.mX[1UL];
  out->mLOG.mX[16UL] = t3->mX.mX[1UL] * 9.5492965855137211;
  out->mLOG.mX[17UL] = t3->mX.mX[0UL];
  out->mLOG.mX[18UL] = t3->mX.mX[2UL];
  out->mLOG.mX[19UL] = motorLF1_Mass_v * 10.0;
  out->mLOG.mX[20UL] = motorLF1_Ideal_Rotational_Motion_Sensor_W;
  out->mLOG.mX[21UL] = t3->mX.mX[2UL];
  out->mLOG.mX[22UL] = t3->mX.mX[2UL];
  out->mLOG.mX[23UL] = motorLF1_Ideal_Rotational_Motion_Sensor_W;
  out->mLOG.mX[24UL] = motorLF1_Mass_v;
  out->mLOG.mX[25UL] = motorLF1_Mass_v;
  out->mLOG.mX[26UL] = motorLF1_Mass_f;
  out->mLOG.mX[27UL] = motorLF1_Mass_v;
  out->mLOG.mX[28UL] = motorLF1_Mass_v * 430.0;
  out->mLOG.mX[29UL] = motorLF1_Mass_v * 10.0;
  out->mLOG.mX[30UL] = motorLF1_Simple_Gear_tB;
  out->mLOG.mX[31UL] = motorLF1_Mass_v * 430.0;
  out->mLOG.mX[33UL] = 0.0;
  out->mLOG.mX[34UL] = motorLF1_Mass_v * 10.0;
  out->mLOG.mX[35UL] = motorLF1_Ideal_Rotational_Motion_Sensor_W;
  out->mLOG.mX[36UL] = 0.0;
  out->mLOG.mX[37UL] = 0.0;
  out->mLOG.mX[38UL] = motorLF1_Simple_Gear_tB * -43.0;
  out->mLOG.mX[39UL] = t3->mU.mX[0UL];
  out->mLOG.mX[40UL] = motorLF1_Mass_v * 10.0;
  out->mLOG.mX[41UL] = motorLF1_Mass_v;
  out->mLOG.mX[42UL] = motorLF1_Mass_v * 10.0;
  out->mLOG.mX[43UL] = motorLF1_Mass_v * 10.0;
  out->mLOG.mX[44UL] = motorLF1_Tire_Simple_inertia_t;
  out->mLOG.mX[46UL] = motorLF1_Mass_v * 10.0;
  out->mLOG.mX[47UL] = motorLF1_Mass_v;
  out->mLOG.mX[48UL] = -motorLF1_Mass_f;
  out->mLOG.mX[49UL] = motorLF1_Mass_f * 0.1;
  out->mLOG.mX[50UL] = t3->mU.mX[0UL];
  out->mLOG.mX[51UL] = t3->mU.mX[0UL];
  out->mLOG.mX[52UL] = t3->mU.mX[0UL];
  (void)sys;
  (void)out;
  return 0;
}
