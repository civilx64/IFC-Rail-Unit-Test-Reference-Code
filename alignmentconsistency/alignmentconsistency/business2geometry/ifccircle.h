#pragma once


#include "ifcaxis2placement2d.h"


static	inline	SdaiInstance    ___CreateCircleInstance(
                                        SdaiModel   model,
                                        double      radiusOfCurvature,
                                        ___MATRIX   * matrix
                                    )
{
    SdaiInstance	ifcCircleInstance = sdaiCreateInstanceBN(model, "IFCCIRCLE");

    sdaiPutAttrBN(ifcCircleInstance, "Position", sdaiINSTANCE, (void*) ___CreateAxis2Placement2DInstance(model, matrix));

    double  radius = std::fabs(radiusOfCurvature);
    sdaiPutAttrBN(ifcCircleInstance, "Radius", sdaiREAL, &radius);

    assert(ifcCircleInstance);
    return	ifcCircleInstance;
}

static	inline	SdaiInteger ___CreateCircleInstance(
                                    SdaiModel   model,
                                    double      radiusOfCurvature
                                )
{
    SdaiInstance	ifcCircleInstance = sdaiCreateInstanceBN(model, "IFCCIRCLE");

    ___MATRIX  matrix;
    ___MatrixIdentity(&matrix);

    if (radiusOfCurvature > 0.) {
        matrix._11 = 0.;
        matrix._12 = -1.;
        assert(matrix._13 == 0.);
    }
    else {
        matrix._11 = 0.;
        matrix._12 = 1.;
        assert(matrix._13 == 0.);
    }

    matrix._42 = radiusOfCurvature;
    sdaiPutAttrBN(ifcCircleInstance, "Position", sdaiINSTANCE, (void*) ___CreateAxis2Placement2DInstance(model, &matrix));

    double  radius = std::fabs(radiusOfCurvature);
    sdaiPutAttrBN(ifcCircleInstance, "Radius", sdaiREAL, &radius);

    assert(ifcCircleInstance);
    return	ifcCircleInstance;
}

static	inline	SdaiInstance    ___CreateCircleInstance(
                                        SdaiModel           model,
                                        const ___VECTOR2    * origin,
                                        double              radius
                                    )
{
    SdaiInstance	ifcCircleInstance = sdaiCreateInstanceBN(model, "IFCCIRCLE");

    ___MATRIX  matrix;
    ___MatrixIdentity(&matrix);
    matrix._41 = origin->u;
    matrix._42 = origin->v;

    sdaiPutAttrBN(ifcCircleInstance, "Position", sdaiINSTANCE, (void*) ___CreateAxis2Placement2DInstance(model, &matrix));

    sdaiPutAttrBN(ifcCircleInstance, "Radius", sdaiREAL, &radius);

    assert(ifcCircleInstance);
    return	ifcCircleInstance;
}

static	inline	double  ___CircularArcLengthMeasureToParameterValue(
                                double      radiusOfCurvature,
                                double      lengthMeasure
                            )
{
    if (radiusOfCurvature)
        return  lengthMeasure / std::fabs(radiusOfCurvature);

    assert(false);
    return  lengthMeasure;
}

static	inline	double  ___CircularArcParameterValueToLengthMeasure(
                                double      radiusOfCurvature,
                                double      parameterValue
                            )
{
    assert(radiusOfCurvature);
    return  parameterValue * std::fabs(radiusOfCurvature);
}
