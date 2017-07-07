#include <cfloat>

#include "RecoLocalCalo/HcalRecAlgos/interface/parseHFPhase1AlgoDescription.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

// Phase 1 HF reco algorithm headers
#include "RecoLocalCalo/HcalRecAlgos/interface/HFSimpleTimeCheck.h"
#include "RecoLocalCalo/HcalRecAlgos/interface/HFFlexibleTimeCheck.h"

std::unique_ptr<AbsHFPhase1Algo>
parseHFPhase1AlgoDescription(const edm::ParameterSet& ps)
{
    std::unique_ptr<AbsHFPhase1Algo> algo;

    const std::string& className = ps.getParameter<std::string>("Class");

    const bool isHFSimpleTimeCheck = className == "HFSimpleTimeCheck";
    if (isHFSimpleTimeCheck || className == "HFFlexibleTimeCheck")
    {
        const std::vector<double>& energyWeightsVec =
            ps.getParameter<std::vector<double> >("energyWeights");
        const unsigned soiPhase =
            ps.getParameter<unsigned>("soiPhase");
        const float timeShift =
            ps.getParameter<double>("timeShift");
        const float triseIfNoTDC =
            ps.getParameter<double>("triseIfNoTDC");
        const float tfallIfNoTDC =
            ps.getParameter<double>("tfallIfNoTDC");
        const bool rejectAllFailures =
            ps.getParameter<bool>("rejectAllFailures");

        float minChargeForUndershoot = -10000.f, minChargeForOvershoot = -10000.f;
        if (ps.existsAs<double>("minChargeForUndershoot"))
            minChargeForUndershoot = ps.getParameter<double>("minChargeForUndershoot");
        if (ps.existsAs<double>("minChargeForOvershoot"))
            minChargeForOvershoot = ps.getParameter<double>("minChargeForOvershoot");

        float energyWeights[2*HFAnodeStatus::N_POSSIBLE_STATES-1][2];
        const unsigned sz = sizeof(energyWeights)/sizeof(energyWeights[0][0]);

        if (energyWeightsVec.size() == sz)
        {
            std::pair<float,float> tlimits[2];
            if (isHFSimpleTimeCheck)
            {
                // Must specify the time limits explicitly for this algorithm
                const std::vector<double>& tlimitsVec =
                    ps.getParameter<std::vector<double> >("tlimits");
                if (tlimitsVec.size() == 4)
                {
                    tlimits[0] = std::pair<float,float>(tlimitsVec[0], tlimitsVec[1]);
                    tlimits[1] = std::pair<float,float>(tlimitsVec[2], tlimitsVec[3]);
                }
                else
                    return algo;
            }
            else
            {
                // Use "all pass" time limits values, just in case
                tlimits[0] = std::pair<float,float>(-FLT_MAX, FLT_MAX);
                tlimits[1] = tlimits[0];
            }

            // Same order of elements as in the natural C array mapping
            float* to = &energyWeights[0][0];
            for (unsigned i=0; i<sz; ++i)
                to[i] = energyWeightsVec[i];

            // Create the algorithm object
            if (isHFSimpleTimeCheck)
                algo = std::unique_ptr<AbsHFPhase1Algo>(
                    new HFSimpleTimeCheck(tlimits, energyWeights, soiPhase,
                                          timeShift, triseIfNoTDC, tfallIfNoTDC,
                                          minChargeForUndershoot, minChargeForOvershoot,
                                          rejectAllFailures));
            else
                algo = std::unique_ptr<AbsHFPhase1Algo>(
                    new HFFlexibleTimeCheck(tlimits, energyWeights, soiPhase,
                                            timeShift, triseIfNoTDC, tfallIfNoTDC,
                                            minChargeForUndershoot, minChargeForOvershoot,
                                            rejectAllFailures));
        }
    }

    return algo;
}
