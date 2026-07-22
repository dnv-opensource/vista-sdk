/**
 * @file decimal.cpp
 * @brief Demonstrates Decimal usage with maritime vessel measurement values
 * @details This sample shows how to use the Vista SDK Decimal type for exact
 *          sensor readings, unit conversions, and calculated metrics typical
 *          of ship monitoring systems (propulsion, fuel, navigation, stability).
 */

#include <dnv/VistaSDK.h>

#include <iomanip>
#include <iostream>
#include <string>

int main()
{
    using namespace dnv::vista::sdk;

    std::cout << "=== vista-sdk Decimal type samples ===\n\n";

    {
        std::cout << "1. Propulsion: Main engine temperatures and pressures\n";
        std::cout << "------------------------------------------------------\n";

        // VIS: 411.1/C101 - Main engine cooling water outlet temperature [°C]
        Decimal coolingWaterOutlet{ "87.3" };
        // VIS: 411.1/C102 - Main engine cooling water inlet temperature [°C]
        Decimal coolingWaterInlet{ "72.1" };
        auto deltaT = coolingWaterOutlet - coolingWaterInlet;

        // VIS: 411.1/C201 - Scavenge air pressure [bar]
        Decimal scavengeAirPressure{ "2.847" };

        // VIS: 411.1/C301 - Exhaust gas temperature after turbocharger [°C]
        Decimal exhaustGasTemp{ "342.6" };

        std::cout << "Cooling water outlet:  " << coolingWaterOutlet << " °C\n";
        std::cout << "Cooling water inlet:   " << coolingWaterInlet << " °C\n";
        std::cout << "Delta T (exact):       " << deltaT << " °C\n";
        std::cout << "Scavenge air pressure: " << scavengeAirPressure << " bar\n";
        std::cout << "Exhaust gas temp:      " << exhaustGasTemp << " °C\n";

        std::cout << "\n";
    }

    {
        std::cout << "2. Fuel: Consumption rate and voyage budget\n";
        std::cout << "---------------------------------------------\n";

        // VIS: 421/C101 - Main engine fuel oil consumption [t/day]
        Decimal fuelConsumptionPerDay{ "45.72" };
        // Voyage duration [days]
        Decimal voyageDays{ "12" };
        auto totalFuel = fuelConsumptionPerDay * voyageDays;

        // VIS: 421/C201 - Fuel oil density at 15°C [kg/m³]
        Decimal fuelDensity{ "0.8914" };
        // Convert total fuel [t] to volume [m³]  (t / density [t/m³])
        auto fuelVolumeM3 = totalFuel / fuelDensity;

        // Exact vs double - demonstrates why Decimal matters for fuel accounting
        double dConsumption{ 45.72 };
        double dDays{ 12.0 };
        double dResult = dConsumption * dDays;

        std::cout << "Fuel consumption:      " << fuelConsumptionPerDay << " t/day\n";
        std::cout << "Voyage duration:       " << voyageDays << " days\n";
        std::cout << "Total fuel (Decimal):  " << totalFuel << " t\n";
        std::cout << "Total fuel (double):   ";
        std::cout << std::fixed << std::setprecision(17) << dResult << "\n";
        std::cout << std::defaultfloat;
        std::cout << "Fuel volume:           " << fuelVolumeM3.round(3) << " m³\n";
        std::cout << "Density scale:         " << static_cast<int>(fuelDensity.scale()) << " decimal places\n";

        std::cout << "\n";
    }

    {
        std::cout << "3. Navigation: Speed, distance and ETA\n";
        std::cout << "-----------------------------------------\n";

        // VIS: 221/C101 - Speed over ground [knots]
        Decimal speedOverGround{ "14.3" };
        // VIS: 221/C201 - Speed through water [knots]
        Decimal speedThroughWater{ "13.8" };
        // Current contribution [knots]
        auto currentEffect = speedOverGround - speedThroughWater;

        // Remaining distance to waypoint [nm]
        Decimal distanceToWaypoint{ "1247.6" };
        // ETA [hours]
        auto etaHours = distanceToWaypoint / speedOverGround;
        // Convert to days and hours
        Decimal hoursPerDay{ "24" };
        auto etaDays = etaHours / hoursPerDay;

        std::cout << "Speed over ground:     " << speedOverGround << " kn\n";
        std::cout << "Speed through water:   " << speedThroughWater << " kn\n";
        std::cout << "Current effect:        " << currentEffect << " kn\n";
        std::cout << "Distance to waypoint:  " << distanceToWaypoint << " nm\n";
        std::cout << "ETA:                   " << etaHours.round(2) << " h  (" << etaDays.round(2) << " days)\n";

        std::cout << "\n";
    }

    {
        std::cout << "4. Stability: Draught, displacement and GM\n";
        std::cout << "--------------------------------------------\n";

        // VIS: 511/C101 - Forward draught [m]
        Decimal draughtFwd{ "7.843" };
        // VIS: 511/C102 - Aft draught [m]
        Decimal draughtAft{ "8.126" };
        // Mean draught [m]
        auto meanDraught = (draughtFwd + draughtAft) / Decimal{ "2" };
        // Trim [m] (positive = trimmed by stern)
        auto trim = draughtAft - draughtFwd;

        // Displacement [t] from hydrostatic tables (read from sensor/table at mean draught)
        Decimal displacement{ "23847.5" };
        // KG (centre of gravity above keel) [m]
        Decimal kg{ "9.214" };
        // KM (metacentre above keel) [m] from stability booklet
        Decimal km{ "10.871" };
        // GM (metacentric height) [m]
        auto gm = km - kg;

        std::cout << "Forward draught:       " << draughtFwd << " m\n";
        std::cout << "Aft draught:           " << draughtAft << " m\n";
        std::cout << "Mean draught:          " << meanDraught << " m\n";
        std::cout << "Trim:                  " << trim << " m (by stern)\n";
        std::cout << "Displacement:          " << displacement << " t\n";
        std::cout << "KG:                    " << kg << " m\n";
        std::cout << "KM:                    " << km << " m\n";
        std::cout << "GM (exact):            " << gm << " m\n";

        std::cout << "\n";
    }

    {
        std::cout << "5. Environmental: Celsius to Kelvin and unit conversions\n";
        std::cout << "----------------------------------------------------------\n";

        // Celsius to Kelvin offset - 273.15 is a known IEEE 754 float artifact trigger
        Decimal celsiusToKelvin{ "273.15" };

        // VIS: 412/C101 - Sea water temperature [°C]
        Decimal seaWaterTempC{ "14.8" };
        auto seaWaterTempK = seaWaterTempC + celsiusToKelvin;

        // VIS: 412/C102 - Outside air temperature [°C]
        Decimal airTempC{ "-3.2" };
        auto airTempK = celsiusToKelvin + airTempC;

        // Knots to m/s: 1 kn = 1852/3600 m/s
        Decimal knotsToMs{ "0.514444" };
        Decimal windSpeed{ "18.5" }; // [kn]  VIS: 412/C301 - Wind speed
        auto windSpeedMs = windSpeed * knotsToMs;

        // bar to kPa: 1 bar = 100 kPa
        Decimal barToKPa{ "100" };
        Decimal barometricPressure{ "1.0132" }; // [bar]  VIS: 412/C401
        auto pressureKPa = barometricPressure * barToKPa;

        // Demonstrate float artifact prevention: 273.15f would give "273.149994..."
        float celsiusOffsetFloat = 273.15f;
        Decimal fromFloat{ celsiusOffsetFloat };
        std::cout << std::boolalpha;
        std::cout << "Celsius -> Kelvin offset (string):  " << celsiusToKelvin << " K\n";
        std::cout << "From float 273.15f (exact?):       " << fromFloat << " K  "
                  << "(== \"273.15\": " << (fromFloat == Decimal{ "273.15" }) << ")\n";
        std::cout << "Sea water temp:    " << seaWaterTempC << " °C = " << seaWaterTempK << " K\n";
        std::cout << "Air temp:          " << airTempC << " °C = " << airTempK << " K\n";
        std::cout << "Wind speed:        " << windSpeed << " kn = " << windSpeedMs.round(3) << " m/s\n";
        std::cout << "Barometric press:  " << barometricPressure << " bar = " << pressureKPa.round(2) << " kPa\n";

        std::cout << "\n";
    }

    {
        std::cout << "6. Rounding modes: Sensor quantization and reporting\n";
        std::cout << "-------------------------------------------------------\n";

        // A raw sensor reading with many decimal places
        // VIS: 411.1/C401 - Shaft RPM [rpm]
        Decimal shaftRpm{ "97.4375" };

        std::cout << "Shaft RPM (raw):       " << shaftRpm << " rpm\n";
        std::cout << "Trunc  (floor abs):    " << shaftRpm.trunc() << " rpm\n";
        std::cout << "Floor  (toward -inf):  " << shaftRpm.floor() << " rpm\n";
        std::cout << "Ceil   (toward +inf):  " << shaftRpm.ceil() << " rpm\n";
        std::cout << "Round  1dp (banker's): " << shaftRpm.round(1) << " rpm\n";
        std::cout << "Round  1dp (ties-away):" << shaftRpm.round(1, Decimal::RoundingMode::ToNearestTiesAway)
                  << " rpm\n";

        // Accumulated flow meter - 1000 small increments of 0.047 t
        // VIS: 421/C301 - Fuel oil flow meter
        Decimal flowIncrement{ "0.047" };
        Decimal flowTotal{ "0" };
        for (int i = 0; i < 1000; ++i)
        {
            flowTotal += flowIncrement;
        }

        std::cout << "Flow total (1000 x 0.047 t): " << flowTotal << " t  "
                  << "(exact: " << std::boolalpha << (flowTotal == Decimal{ "47" }) << ")\n";

        std::cout << "\n";
    }

    return 0;
}
