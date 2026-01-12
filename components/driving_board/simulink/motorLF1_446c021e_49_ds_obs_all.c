/* Simscape target specific file.
 * This file is generated for the Simscape network associated with the solver block 'motorLF1/motorLF1/Solver Configuration'.
 */

#include "ne_ds.h"
#include "motorLF1_446c021e_49_ds_sys_struct.h"
#include "motorLF1_446c021e_49_ds_obs_all.h"
#include "motorLF1_446c021e_49_ds.h"
#include "motorLF1_446c021e_49_ds_externals.h"
#include "motorLF1_446c021e_49_ds_external_struct.h"
#include "ssc_ml_fun.h"

int32_T motorLF1_446c021e_49_ds_obs_all(const NeDynamicSystem *sys, const
  NeDynamicSystemInput *t3, NeDsMethodOutput *out)
{
  real_T motorLF1_Controlled_Voltage_Source_i;
  real_T motorLF1_Ideal_Rotational_Motion_Sensor_W;
  real_T motorLF1_Mass_f;
  real_T motorLF1_Mass_v;
  real_T motorLF1_Simple_Gear_tB;
  real_T motorLF1_Tire_Simple_inertia_t;
  out->mOBS_ALL.mX[49UL] = 300.0;
  motorLF1_Controlled_Voltage_Source_i = (t3->mX.mX[0UL] * -0.99999999952338436
    + t3->mX.mX[1UL] * 4.7487132283774637E-11) + t3->mU.mX[0UL] *
    -9.9999999952338438E-10;
  motorLF1_Mass_f = (t3->mX.mX[0UL] * 8.6298642016208227 + t3->mX.mX[1UL] *
                     -0.10522200756069462) + t3->mU.mX[0UL] *
    8.6298642016208239E-9;
  motorLF1_Tire_Simple_inertia_t = (t3->mX.mX[0UL] * 0.0726346903636419 +
    t3->mX.mX[1UL] * -0.00088561856363584624) + t3->mU.mX[0UL] *
    7.2634690363641915E-11;
  out->mOBS_ALL.mX[25UL] = motorLF1_Mass_f * -0.0023255813953488372 +
    motorLF1_Tire_Simple_inertia_t * -0.023255813953488372;
  motorLF1_Mass_v = t3->mX.mX[1UL] * 0.0023255813953488372;
  motorLF1_Ideal_Rotational_Motion_Sensor_W = motorLF1_Mass_v * 10.0;
  out->mOBS_ALL.mX[54UL] = motorLF1_Mass_v * 430.0;
  motorLF1_Simple_Gear_tB = motorLF1_Mass_f * 0.0023255813953488372 +
    motorLF1_Tire_Simple_inertia_t * 0.023255813953488372;
  out->mOBS_ALL.mX[69UL] = t3->mX.mX[1UL] * 0.023255813953488372;
  out->mOBS_ALL.mX[0UL] = motorLF1_Controlled_Voltage_Source_i;
  out->mOBS_ALL.mX[1UL] = 0.0;
  out->mOBS_ALL.mX[2UL] = t3->mU.mX[0UL];
  out->mOBS_ALL.mX[3UL] = t3->mU.mX[0UL];
  out->mOBS_ALL.mX[4UL] = t3->mU.mX[0UL];
  out->mOBS_ALL.mX[5UL] = -motorLF1_Controlled_Voltage_Source_i;
  out->mOBS_ALL.mX[6UL] = -motorLF1_Controlled_Voltage_Source_i;
  out->mOBS_ALL.mX[7UL] = t3->mU.mX[0UL];
  out->mOBS_ALL.mX[8UL] = t3->mU.mX[0UL];
  out->mOBS_ALL.mX[9UL] = -motorLF1_Controlled_Voltage_Source_i;
  out->mOBS_ALL.mX[10UL] = 0.0;
  out->mOBS_ALL.mX[11UL] = 0.0;
  out->mOBS_ALL.mX[12UL] = motorLF1_Mass_v * 430.0;
  out->mOBS_ALL.mX[13UL] = 1.0;
  out->mOBS_ALL.mX[14UL] = 0.0;
  out->mOBS_ALL.mX[15UL] = 0.0;
  out->mOBS_ALL.mX[16UL] = 1.0;
  out->mOBS_ALL.mX[17UL] = 0.0;
  out->mOBS_ALL.mX[18UL] = 0.0;
  out->mOBS_ALL.mX[19UL] = 0.0;
  out->mOBS_ALL.mX[20UL] = -motorLF1_Controlled_Voltage_Source_i;
  out->mOBS_ALL.mX[21UL] = 0.0;
  out->mOBS_ALL.mX[22UL] = 0.0;
  out->mOBS_ALL.mX[23UL] = t3->mU.mX[0UL];
  out->mOBS_ALL.mX[24UL] = motorLF1_Controlled_Voltage_Source_i *
    motorLF1_Controlled_Voltage_Source_i * 476.61557223563062 * 1.0E-6 * 1000.0;
  out->mOBS_ALL.mX[26UL] = t3->mU.mX[0UL];
  out->mOBS_ALL.mX[27UL] = 0.0;
  out->mOBS_ALL.mX[28UL] = t3->mX.mX[1UL];
  out->mOBS_ALL.mX[29UL] = t3->mX.mX[1UL] * 9.5492965855137211;
  out->mOBS_ALL.mX[30UL] = t3->mX.mX[0UL];
  out->mOBS_ALL.mX[31UL] = 0.0;
  out->mOBS_ALL.mX[32UL] = t3->mX.mX[2UL];
  out->mOBS_ALL.mX[33UL] = 0.0;
  out->mOBS_ALL.mX[34UL] = motorLF1_Mass_v * 10.0;
  out->mOBS_ALL.mX[35UL] = motorLF1_Ideal_Rotational_Motion_Sensor_W;
  out->mOBS_ALL.mX[36UL] = 0.0;
  out->mOBS_ALL.mX[37UL] = t3->mX.mX[2UL];
  out->mOBS_ALL.mX[38UL] = t3->mX.mX[2UL];
  out->mOBS_ALL.mX[39UL] = motorLF1_Ideal_Rotational_Motion_Sensor_W;
  out->mOBS_ALL.mX[40UL] = motorLF1_Mass_v;
  out->mOBS_ALL.mX[41UL] = motorLF1_Mass_v;
  out->mOBS_ALL.mX[42UL] = motorLF1_Mass_f;
  out->mOBS_ALL.mX[43UL] = motorLF1_Mass_v;
  out->mOBS_ALL.mX[44UL] = 0.0;
  out->mOBS_ALL.mX[45UL] = motorLF1_Mass_v * 430.0;
  out->mOBS_ALL.mX[46UL] = motorLF1_Mass_v * 10.0;
  out->mOBS_ALL.mX[47UL] = 0.0;
  out->mOBS_ALL.mX[48UL] = motorLF1_Simple_Gear_tB;
  out->mOBS_ALL.mX[50UL] = 0.0;
  out->mOBS_ALL.mX[51UL] = 0.0;
  out->mOBS_ALL.mX[52UL] = motorLF1_Mass_v * 430.0;
  out->mOBS_ALL.mX[53UL] = 0.0;
  out->mOBS_ALL.mX[55UL] = 0.0;
  out->mOBS_ALL.mX[56UL] = motorLF1_Mass_v * 10.0;
  out->mOBS_ALL.mX[57UL] = 0.0;
  out->mOBS_ALL.mX[58UL] = motorLF1_Ideal_Rotational_Motion_Sensor_W;
  out->mOBS_ALL.mX[59UL] = 0.0;
  out->mOBS_ALL.mX[60UL] = motorLF1_Simple_Gear_tB * -43.0;
  out->mOBS_ALL.mX[61UL] = 0.0;
  out->mOBS_ALL.mX[62UL] = 0.0;
  out->mOBS_ALL.mX[63UL] = t3->mU.mX[0UL];
  out->mOBS_ALL.mX[64UL] = motorLF1_Mass_v * 10.0;
  out->mOBS_ALL.mX[65UL] = motorLF1_Mass_v;
  out->mOBS_ALL.mX[66UL] = motorLF1_Mass_v * 10.0;
  out->mOBS_ALL.mX[67UL] = motorLF1_Mass_v * 10.0;
  out->mOBS_ALL.mX[68UL] = motorLF1_Tire_Simple_inertia_t;
  out->mOBS_ALL.mX[70UL] = motorLF1_Mass_v * 10.0;
  out->mOBS_ALL.mX[71UL] = motorLF1_Mass_v;
  out->mOBS_ALL.mX[72UL] = -motorLF1_Mass_f;
  out->mOBS_ALL.mX[73UL] = motorLF1_Mass_f * 0.1;
  out->mOBS_ALL.mX[74UL] = t3->mU.mX[0UL];
  out->mOBS_ALL.mX[75UL] = 0.0;
  out->mOBS_ALL.mX[76UL] = t3->mU.mX[0UL];
  out->mOBS_ALL.mX[77UL] = t3->mU.mX[0UL];
  (void)sys;
  (void)out;
  return 0;
}
