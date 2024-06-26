#pragma once


#include "generic.h"
#include "mathematics.h"
#include "ifcpolynomialcurve.h"
#include "ifcproductdefinitionshape.h"


enum class enum_segment_type : unsigned char
{
    CIRCULARARC,
	CLOTHOID,
    CONSTANTGRADIENT,
    PARABOLICARC
};

static  inline  int_t   ___SegmentCount__alignmentVertical(
                                SdaiInstance    ifcVerticalAlignmentInstance
                            )
{
	SdaiAggr	aggrSegments = nullptr;

    sdaiGetAttrBN(ifcVerticalAlignmentInstance, "Segments", sdaiAGGR, &aggrSegments);

    return  sdaiGetMemberCount(aggrSegments);
}

static  inline  void    SetCurveSegmentTransition4Vertical(
                                SdaiInstance        ifcCurveSegmentInstance,
                                const char          * predefinedTypeCurrentSegment,
                                const char          * predefinedTypePreviousSegment,
                                double              startRadiusCurrentSegment,
                                double              endRadiusPreviousSegment,
                                double              startGradientCurrentSegment,
                                double              endGradientPreviousSegment
                            )
{
    assert(predefinedTypeCurrentSegment);

    //
    //  IfcTransitionCode
    //      CONTINUOUS
    //      CONTSAMEGRADIENT
    //      CONTSAMEGRADIENTSAMECURVATURE
    //      DISCONTINUOUS
    //
    if (predefinedTypePreviousSegment == nullptr) {
///        char    transitionCode[] = "DISCONTINUOUS";
        char    transitionCode[] = "CONTINUOUS";
        sdaiPutAttrBN(ifcCurveSegmentInstance, "Transition", sdaiENUM, (void*) transitionCode);
    }
    else {
        if (___equals(predefinedTypeCurrentSegment, "CONSTANTGRADIENT") && ___equals(predefinedTypePreviousSegment, "CONSTANTGRADIENT") &&
            (startGradientCurrentSegment != endGradientPreviousSegment)) {
            char    transitionCode[] = "CONTINUOUS";
            sdaiPutAttrBN(ifcCurveSegmentInstance, "Transition", sdaiENUM, (void*) transitionCode);
        }
        else {
            if (startRadiusCurrentSegment == endRadiusPreviousSegment) {
                char    transitionCode[] = "CONTSAMEGRADIENTSAMECURVATURE";
                sdaiPutAttrBN(ifcCurveSegmentInstance, "Transition", sdaiENUM, (void*) transitionCode);
            }
            else {
                char    transitionCode[] = "CONTSAMEGRADIENT";
                sdaiPutAttrBN(ifcCurveSegmentInstance, "Transition", sdaiENUM, (void*) transitionCode);
            }
        }
    }
}

static  inline  SdaiInstance    ___CreateGradientCurve__alignmentVertical(
                                        SdaiModel       model,
                                        SdaiInstance    ifcVerticalAlignmentInstance,
                                        double          startDistAlongHorizontalAlignment
                                    )
{
    bool    isIFC4X3_ADD1 = false;
#ifdef _DEBUG
    double  epsilon = 0.0000001;
#endif // _DEBUG

    int_t   noSegmentInstances =
                ___GetAlignmentSegments(
                        model,
                        ifcVerticalAlignmentInstance,
                        nullptr
                    );

    if (noSegmentInstances) {
        SdaiInstance	ifcGradientCurveInstance = sdaiCreateInstanceBN(model, "IFCGRADIENTCURVE");
	    SdaiAggr        aggrCurveSegment = sdaiCreateAggrBN(ifcGradientCurveInstance, "Segments");
	    char    selfIntersect[2] = "F";
	    sdaiPutAttrBN(ifcGradientCurveInstance, "SelfIntersect", sdaiENUM, (void*) selfIntersect);

        SdaiInstance    * segmentInstances = new SdaiInstance[noSegmentInstances];

        ___GetAlignmentSegments(
                model,
                ifcVerticalAlignmentInstance,
                segmentInstances
            );

        double               * pStartGradient = new double[noSegmentInstances],
                             * pEndGradient = new double[noSegmentInstances],
                             * pStartHeight = new double[noSegmentInstances],
                             * pRadiusOfCurvature = new double[noSegmentInstances];
        enum_segment_type    * pSegmentType = new enum_segment_type[noSegmentInstances];

        for (int_t index = 0; index < noSegmentInstances; index++) {
            SdaiInstance    ifcAlignmentSegmentInstance = segmentInstances[index];
            assert(sdaiGetInstanceType(ifcAlignmentSegmentInstance) == sdaiGetEntity(model, "IFCALIGNMENTSEGMENT"));

            SdaiInstance    ifcAlignmentVerticalSegmentInstance = 0;
            sdaiGetAttrBN(ifcAlignmentSegmentInstance, "DesignParameters", sdaiINSTANCE, (void*) &ifcAlignmentVerticalSegmentInstance);

            //
            //  StartHeight
            //
            double  startHeight = 0.;
            sdaiGetAttrBN(ifcAlignmentVerticalSegmentInstance, "StartHeight", sdaiREAL, &startHeight);
            pStartHeight[index] = startHeight;

            char    * predefinedType = nullptr;
            sdaiGetAttrBN(ifcAlignmentVerticalSegmentInstance, "PredefinedType", sdaiENUM, &predefinedType);
            if (___equals(predefinedType, "CIRCULARARC")) {
                //
                //  StartGradient
                //
                double  startGradient__ = 0.;
                sdaiGetAttrBN(ifcAlignmentVerticalSegmentInstance, "StartGradient", sdaiREAL, &startGradient__);
                double  startAngle = std::atan(startGradient__);
                pStartGradient[index] = startGradient__;

                //
                //  EndGradient
                //
                double  endGradient__ = 0.;
                sdaiGetAttrBN(ifcAlignmentVerticalSegmentInstance, "EndGradient", sdaiREAL, &endGradient__);
                double  endAngle = std::atan(endGradient__);
                pEndGradient[index] = endGradient__;

                double  horizontalLength = 0.;
                sdaiGetAttrBN(ifcAlignmentVerticalSegmentInstance, "HorizontalLength", sdaiREAL, &horizontalLength);

                pRadiusOfCurvature[index] = horizontalLength / (sin(endAngle) - sin(startAngle));
                pSegmentType[index] = enum_segment_type::CIRCULARARC;
            }
            else if (___equals(predefinedType, "CLOTHOID")) {
                pStartGradient[index]	  = 0.;
                pEndGradient[index] 	  = 0.;

                pRadiusOfCurvature[index] = 0.;
                pSegmentType[index]       = enum_segment_type::CLOTHOID;
            }
            else if (___equals(predefinedType, "CONSTANTGRADIENT")) {
                double  startGradient__ = 0.;
                sdaiGetAttrBN(ifcAlignmentVerticalSegmentInstance, "StartGradient", sdaiREAL, &startGradient__);
//                double  angle = std::atan(startGradient__);
                pStartGradient[index]	  = startGradient__;
                pEndGradient[index] 	  = startGradient__;

                pRadiusOfCurvature[index] = 0.;
                pSegmentType[index]       = enum_segment_type::CONSTANTGRADIENT;
            }
            else {
                assert(___equals(predefinedType, "PARABOLICARC"));

                pStartGradient[index]	  = 0.;
                pEndGradient[index]  	  = 0.;

                pRadiusOfCurvature[index] = 0.;
                pSegmentType[index]       = enum_segment_type::PARABOLICARC;
            }
        }
        
        double          mostRecentStartDistAlong       = 0.,
                        mostRecentHorizontalLength     = 0.,
                        mostRecentEndGradient          = 0.;
        SdaiInstance    mostRecentCurveSegmentInstance = 0;
#ifdef _DEBUG
        ___VECTOR2  mostRecentLocation = { 0., 0. };
#endif // _DEBUG

#ifdef _DEBUG
        ___POINT4D  previousEndPnt = { { 0., 0., 0. }, { 0., 0., 0. }, { 0., 0., 0.} };
#endif // _DEBUG

        const char  * predefinedTypePreviousSegment = nullptr;
        double      endGradientPreviousSegment		= 0.,
                    endRadiusPreviousSegment		= 0.;

        for (int_t index = 0; index < noSegmentInstances; index++) {
            SdaiInstance    ifcAlignmentSegmentInstance = segmentInstances[index];
            assert(sdaiGetInstanceType(ifcAlignmentSegmentInstance) == sdaiGetEntity(model, "IFCALIGNMENTSEGMENT"));

            SdaiInstance    ifcAlignmentVerticalSegmentInstance = 0;
            sdaiGetAttrBN(ifcAlignmentSegmentInstance, "DesignParameters", sdaiINSTANCE, (void*) &ifcAlignmentVerticalSegmentInstance);
            assert(sdaiGetInstanceType(ifcAlignmentVerticalSegmentInstance) == sdaiGetEntity(model, "IFCALIGNMENTVERTICALSEGMENT"));

            {
                SdaiInstance    ifcCurveSegmentInstance = sdaiCreateInstanceBN(model, "IFCCURVESEGMENT");

                //
                //  Add geometry for Ifc...Alignment...
                //
                sdaiPutAttrBN(
                        ifcAlignmentSegmentInstance,
                        "ObjectPlacement",
                        sdaiINSTANCE,
                        (void*) ___CreateObjectPlacement(
                                        model
                                    )
                    );
					
                assert(ifcCurveSegmentInstance && ifcAlignmentSegmentInstance);
                sdaiPutAttrBN(
                        ifcAlignmentSegmentInstance,
                        "Representation",
                        sdaiINSTANCE,
                        (void*) ___CreateProductDefinitionShapeInstance(
                                        model,
                                        ifcCurveSegmentInstance,
                                        "Segment"
                                    )
                    );

                //
                //  ENTITY IfcAlignmentVerticalSegment
                //      StartDistAlong      IfcLengthMeasure
                //      HorizontalLength    IfcPositiveLengthMeasure
                //      StartHeight         IfcLengthMeasure
                //      StartGradient       IfcLengthMeasure
                //      EndGradient         IfcLengthMeasure
                //      RadiusOfCurvature   OPTIONAL IfcPositiveLengthMeasure
                //      PredefinedType      IfcAlignmentVerticalSegmentTypeEnum
                //  END_ENTITY
                //

                //
                //  StartDistAlong
                //
                double  startDistAlong = 0.;
                sdaiGetAttrBN(ifcAlignmentVerticalSegmentInstance, "StartDistAlong", sdaiREAL, &startDistAlong);

                //
                //  HorizontalLength
                //
                double  horizontalLength = 0.;
                sdaiGetAttrBN(ifcAlignmentVerticalSegmentInstance, "HorizontalLength", sdaiREAL, &horizontalLength);

                //
                //  StartHeight
                //
                double  startHeight = 0.;
                sdaiGetAttrBN(ifcAlignmentVerticalSegmentInstance, "StartHeight", sdaiREAL, &startHeight);

                //
                //  StartGradient
                //
                double  startGradient__ = 0.;
                sdaiGetAttrBN(ifcAlignmentVerticalSegmentInstance, "StartGradient", sdaiREAL, &startGradient__);

                //
                //  EndGradient
                //
                double  endGradient__ = 0.;
                sdaiGetAttrBN(ifcAlignmentVerticalSegmentInstance, "EndGradient", sdaiREAL, &endGradient__);

                const char    * predefinedType = nullptr;
                sdaiGetAttrBN(ifcAlignmentVerticalSegmentInstance, "PredefinedType", sdaiENUM, &predefinedType);

                if (___equals(predefinedType, "PARABOLICARC")) {
                    if (startGradient__ == 0. && index) {
                        startGradient__ = pEndGradient[index - 1];
                    }
                    if (endGradient__ == 0. && index < noSegmentInstances - 1) {
                        endGradient__ = pStartGradient[index + 1];
                    }
                }

                mostRecentStartDistAlong   = startDistAlong;
                mostRecentHorizontalLength = horizontalLength;
                mostRecentEndGradient      = endGradient__;

                //
                //  RadiusOfCurvature
                //
                double  radiusOfCurvature = 0.;
                sdaiGetAttrBN(ifcAlignmentVerticalSegmentInstance, "RadiusOfCurvature", sdaiREAL, &radiusOfCurvature);

                ___VECTOR2  refDirection = {
                                    1.,
                                    startGradient__
                                },
                            location = {
                                    startDistAlong - startDistAlongHorizontalAlignment,
                                    startHeight
                                };
                ___Vec2Normalize(&refDirection);
                sdaiPutAttrBN(ifcCurveSegmentInstance, "Placement", sdaiINSTANCE, (void*) ___CreateAxis2Placement2DInstance(model, &location, &refDirection));
#ifdef _DEBUG
                mostRecentLocation.u = location.u;
                mostRecentLocation.v = location.v;
#endif // _DEBUG

                double  heightDeviation = 0.;

                double  ___startRadius = 0.,
                        ___endRadius = 0.;

                //
                //  Parse the individual segments
                //      CONSTANTGRADIENT
                //      CIRCULARARC
                //      PARABOLICARC
                //      CLOTHOID
                //
                if (___equals(predefinedType, "CIRCULARARC")) {
                    double  startAngle = std::atan(startGradient__),
                            endAngle = std::atan(endGradient__);
                    assert(startAngle > -___Pi && startAngle < ___Pi && endAngle > -___Pi && endAngle < ___Pi);

                    ___startRadius = ___endRadius = radiusOfCurvature;

                    double      radius;
                    ___VECTOR2  origin;
                    if (startAngle < endAngle) {
 ///                       assert(radiusOfCurvature < 0.);
                        //
                        //  Ox = -sin( startAngle ) * radius         Ox = horizontalLength - sin( endAngle ) * radius
                        //  Oy = cos( startAngle ) * radius          Oy = offsetY + cos( endAngle ) * radius
                        //
                        //  horizontalLength = (sin( endAngle ) - sin( startAngle )) * radius
                        //  radius = horizontalLength / (sin( endAngle ) - sin( startAngle ));
                        //
                        radius = horizontalLength / (sin(endAngle) - sin(startAngle));
 ///////////////////////////////////!!!!!!!!!!!!!!!!!!!!!!!!!!!!!????????????????????                       assert(radius > 0. && std::fabs(radius - radiusOfCurvature) < 0.001);

                        //
                        //  heightDeviation = (cos( startAngle ) - cos( endAngle )) * radius;
                        //
                        heightDeviation = (cos(startAngle) - cos(endAngle)) * radius;

                        origin.u = - sin(startAngle) * radius;
                        origin.v =   cos(startAngle) * radius;

                        assert(std::fabs(origin.u - (horizontalLength - sin(endAngle) * radius)) < epsilon);
                        assert(std::fabs(origin.v - (heightDeviation  + cos(endAngle) * radius)) < epsilon);

                        startAngle += 3. * ___Pi / 2.;
                        endAngle += 3. * ___Pi / 2.;

//                        origin.x = -cos(startAngle) * radius;
//                        origin.y = -sin(startAngle) * radius;
                    }
                    else {
                        assert(startAngle > endAngle);
///                        assert(radiusOfCurvature > 0.);
                        //
                        //  Ox = sin( startAngle ) * radius         Ox = horizontalLength + sin( endAngle ) * radius
                        //  Oy = -cos( startAngle ) * radius        Oy = offsetY - cos( endAngle ) * radius
                        //
                        //  horizontalLength = (sin( startAngle ) - sin( endAngle )) * radius
                        //  radius = horizontalLength / (sin( startAngle ) - sin( endAngle ));
                        //
                        radius = horizontalLength / (sin(startAngle) - sin(endAngle));
///                        assert(radius > 0. && std::fabs(radius + radiusOfCurvature) < 0.001);
 
                        //
                        //  heightDeviation = (cos( endAngle ) - cos( startAngle )) * radius;
                        //
                        heightDeviation = (cos(endAngle) - cos(startAngle)) * radius;

                        origin.u =   sin(startAngle) * radius;
                        origin.v = - cos(startAngle) * radius;

                        assert(std::fabs(origin.u - (horizontalLength + sin(endAngle) * radius)) < epsilon);
                        assert(std::fabs(origin.v - (heightDeviation  - cos(endAngle) * radius)) < epsilon);

                        startAngle += ___Pi / 2.;
                        endAngle += ___Pi / 2.;

                        origin.u = - cos(startAngle) * radius;
                        origin.v = - sin(startAngle) * radius;
                    }

                    ___MATRIX   myMatrix;
                    ___MatrixIdentity(&myMatrix);
                    myMatrix._41 = origin.u;
                    myMatrix._42 = origin.v;

                    SdaiInstance    ifcCircularArcParentCurve =
                                        ___CreateCircleInstance(
                                                model,
                                                radius,
                                                &myMatrix
                                            );
                    sdaiPutAttrBN(ifcCurveSegmentInstance, "ParentCurve", sdaiINSTANCE, (void*) ifcCircularArcParentCurve);

                    double  offset =
                                ___CircularArcParameterValueToLengthMeasure(
                                        radius,
                                        startAngle
                                    ),
                            segmentLength =
                                ___CircularArcParameterValueToLengthMeasure(
                                        radius,
                                        endAngle - startAngle
                                    );
                    if (isIFC4X3_ADD1) {
                        if (offset >= 0. && segmentLength >= 0. &&
                            forceUseParameterValue == false) {
                            //
                            //  SegmentStart
                            //
                            double  segmentStart = offset;

                            void   * segmentStartADB = sdaiCreateADB(sdaiREAL, &segmentStart);
                            sdaiPutADBTypePath(segmentStartADB, 1, "IFCNONNEGATIVELENGTHMEASURE");
                            sdaiPutAttrBN(ifcCurveSegmentInstance, "SegmentStart", sdaiADB, (void*) segmentStartADB);

                            //
                            //  SegmentLength
                            //
                            void   * segmentLengthADB = sdaiCreateADB(sdaiREAL, &segmentLength);
                            sdaiPutADBTypePath(segmentLengthADB, 1, "IFCNONNEGATIVELENGTHMEASURE");
                            sdaiPutAttrBN(ifcCurveSegmentInstance, "SegmentLength", sdaiADB, (void*) segmentLengthADB);
                        }
                        else {
                            //
                            //  SegmentStart
                            //
                            double  segmentStartParameterValue = startAngle;
                          	assert(___CircularArcLengthMeasureToParameterValue(radius, offset) == segmentStartParameterValue);

                            void   * segmentStartADB = sdaiCreateADB(sdaiREAL, &segmentStartParameterValue);
                            sdaiPutADBTypePath(segmentStartADB, 1, "IFCPARAMETERVALUE");
                            sdaiPutAttrBN(ifcCurveSegmentInstance, "SegmentStart", sdaiADB, (void*) segmentStartADB);

                            //
                            //  SegmentLength
                            //
                            double  segmentLengthParameterValue = endAngle - startAngle;
	                        assert(___CircularArcLengthMeasureToParameterValue(radius, segmentLength) == segmentLengthParameterValue);

                            void   * segmentLengthADB = sdaiCreateADB(sdaiREAL, &segmentLengthParameterValue);
                            sdaiPutADBTypePath(segmentLengthADB, 1, "IFCPARAMETERVALUE");
                            sdaiPutAttrBN(ifcCurveSegmentInstance, "SegmentLength", sdaiADB, (void*) segmentLengthADB);
                        }
                    }
                    else {
                        //
                        //  SegmentStart
                        //
                        double  segmentStart = offset;

                        void   * segmentStartADB = sdaiCreateADB(sdaiREAL, &segmentStart);
                        sdaiPutADBTypePath(segmentStartADB, 1, "IFCLENGTHMEASURE");
                        sdaiPutAttrBN(ifcCurveSegmentInstance, "SegmentStart", sdaiADB, (void*) segmentStartADB);

                        //
                        //  SegmentLength
                        //
                        void   * segmentLengthADB = sdaiCreateADB(sdaiREAL, &segmentLength);
                        sdaiPutADBTypePath(segmentLengthADB, 1, "IFCLENGTHMEASURE");
                        sdaiPutAttrBN(ifcCurveSegmentInstance, "SegmentLength", sdaiADB, (void*) segmentLengthADB);
                    }

                    double  radiusOfCurvature__ = 0.;
                    sdaiGetAttrBN(ifcAlignmentVerticalSegmentInstance, "RadiusOfCurvature", sdaiREAL, &radiusOfCurvature__);

                    if (radiusOfCurvature__ == 0. && radius) {
                        double  radiusOfCurvature = radius;
                        sdaiPutAttrBN(ifcAlignmentVerticalSegmentInstance, "RadiusOfCurvature", sdaiREAL, &radiusOfCurvature);
                    }
                }
                else if (___equals(predefinedType, "CLOTHOID")) {
                    double  startAngle = std::atan(startGradient__),
                            endAngle = std::atan(endGradient__);

                    {
                        //
                        //  new definition where the context defines the radius
                        //
                        double  startRadiusOfCurvature = index ? pRadiusOfCurvature[index - 1] : pRadiusOfCurvature[index];
#ifdef _DEBUG
                        double  endRadiusOfCurvature = (index + 1 < noSegmentInstances) ? pRadiusOfCurvature[index + 1] : pRadiusOfCurvature[index];
#endif // _DEBUG

                        ___startRadius = startRadiusOfCurvature;
                        ___endRadius   = (index + 1 < noSegmentInstances) ? pRadiusOfCurvature[index + 1] : pRadiusOfCurvature[index];

                        //
                        //  HorizontalLength
                        //
                        double  horizontalLength__ = 0.;
                        sdaiGetAttrBN(ifcAlignmentVerticalSegmentInstance, "HorizontalLength", sdaiREAL, &horizontalLength__);
                        assert(horizontalLength__ == horizontalLength);

                        if (startRadiusOfCurvature == 0.) {
//                            double  gradientSpiral = pEndGradient[i - 1];

                            ___VECTOR3 	originSpiral = { 0., 0., 0. },
                                    	correctedEndPoint;

#ifdef _DEBUG
                            ___VECTOR3		refDirectionSpiral = { 1., startGradient__, 0. },
                                    		endPoint = { horizontalLength, pStartHeight[index + 1] - pStartHeight[index], 0. };
                            double	D = ___PointLineDistance(&correctedEndPoint, &endPoint, &originSpiral, &refDirectionSpiral);
#endif // _DEBUG

                            double  correctedHorizontalLength = ___Vec3Distance(&originSpiral, &correctedEndPoint),
                                    correctedStartAngle = 0.,
                                    correctedEndAngle = endAngle - startAngle;

                            double  L1 = correctedStartAngle ? (correctedStartAngle / std::fabs(correctedStartAngle)) * sqrt(2. * std::fabs(correctedStartAngle) / 1.) : 0.,
                                    L2 = correctedEndAngle ? (correctedEndAngle / std::fabs(correctedEndAngle)) * sqrt(1. * std::fabs(correctedEndAngle) / 1.) : 0.;

                            double  polynomialConstants[3] = { 0., 0., 1. },
                                    x1 = ___IntegralTaylorSeriesCos(polynomialConstants, 3, L1),
                                    x2 = ___IntegralTaylorSeriesCos(polynomialConstants, 3, L2);

                            {
                                double  distance = x2 - x1;
//                                assert(distance > 0.);

                                double  a = std::pow(distance / correctedHorizontalLength, 2);
                                polynomialConstants[2] = a;
                                L1 = correctedStartAngle ? (correctedStartAngle / std::fabs(correctedStartAngle)) * sqrt(2. * std::fabs(correctedStartAngle) / a) : 0.;
                                L2 = correctedEndAngle ? (correctedEndAngle / std::fabs(correctedEndAngle)) * sqrt(1. * std::fabs(correctedEndAngle) / a) : 0.;
//                                assert(L1 < L2);

                                x1 = ___IntegralTaylorSeriesCos(polynomialConstants, 3, L1);
                                x2 = ___IntegralTaylorSeriesCos(polynomialConstants, 3, L2);
//                                double  dist = x2 - x1;
    //                            assert(std::fabs(dist - correctedHorizontalLength) < 0.0000001);

#ifdef _DEBUG
                                double  y2 = ___IntegralTaylorSeriesSin(polynomialConstants, 3, L2);
#endif // _DEBUG
                                assert(std::fabs(std::fabs(y2) - std::fabs(D)) < 0.0000001);

                                double  segmentLength = L2 - L1,
                                        offset = L1;

                                double  linearTerm = (correctedEndAngle / std::fabs(correctedEndAngle)) / (2. * a);

#ifdef _DEBUG
                                double  angle1 = ___AngleByAngleDeviationPolynomial(0., 0., (linearTerm) ? 1. / linearTerm : 0., 0., L1);
                                double  angle2 = ___AngleByAngleDeviationPolynomial(0., 0., (linearTerm) ? 1. / linearTerm : 0., 0., L2);
#endif // _DEBUG
                                assert(std::fabs(correctedStartAngle - angle1) < 0.000001);
                                assert(std::fabs(correctedEndAngle - angle2) < 0.000001);

                                int_t   ifcClothoidInstance =
                                            ___CreateClothoidInstance(
                                                    model,
//                                                    linearTerm ? segmentLength * pow(std::fabs(linearTerm), -1. / 2.) * linearTerm / std::fabs(linearTerm) : 0.
                                                    linearTerm ? 1. * pow(std::fabs(linearTerm), -1. / 2.) * linearTerm / std::fabs(linearTerm) : 0.,
                                                    nullptr
                                               );
                                sdaiPutAttrBN(ifcCurveSegmentInstance, "ParentCurve", sdaiINSTANCE, (void*) ifcClothoidInstance);

                                if (isIFC4X3_ADD1) {
                                    if (offset >= 0. && segmentLength >= 0. &&
                                        forceUseParameterValue == false) {
                                        //
                                        //  SegmentStart
                                        //
                                        double  segmentStart = offset;

                                        void   * segmentStartADB = sdaiCreateADB(sdaiREAL, &segmentStart);
                                        sdaiPutADBTypePath(segmentStartADB, 1, "IFCNONNEGATIVELENGTHMEASURE");
                                        sdaiPutAttrBN(ifcCurveSegmentInstance, "SegmentStart", sdaiADB, (void*) segmentStartADB);

                                        //
                                        //  SegmentLength
                                        //
                                        void   * segmentLengthADB = sdaiCreateADB(sdaiREAL, &segmentLength);
                                        sdaiPutADBTypePath(segmentLengthADB, 1, "IFCNONNEGATIVELENGTHMEASURE");
                                        sdaiPutAttrBN(ifcCurveSegmentInstance, "SegmentLength", sdaiADB, (void*) segmentLengthADB);
                                    }
                                    else {
                                        //
                                        //  SegmentStart
                                        //
                                        double  segmentStartParameterValue =
                                                    ___ClothoidLengthMeasureToParameterValue(
                                                            linearTerm ? segmentLength * pow(std::fabs(linearTerm), -1. / 2.) * linearTerm / std::fabs(linearTerm) : 0.,
                                                            offset
                                                        );

                                        void   * segmentStartADB = sdaiCreateADB(sdaiREAL, &segmentStartParameterValue);
                                        sdaiPutADBTypePath(segmentStartADB, 1, "IFCPARAMETERVALUE");
                                        sdaiPutAttrBN(ifcCurveSegmentInstance, "SegmentStart", sdaiADB, (void*) segmentStartADB);

                                        //
                                        //  SegmentLength
                                        //
                                        double  segmentLengthParameterValue =
                                                    ___ClothoidLengthMeasureToParameterValue(
                                                            linearTerm ? segmentLength * pow(std::fabs(linearTerm), -1. / 2.) * linearTerm / std::fabs(linearTerm) : 0.,
                                                            segmentLength
                                                        );

                                        void   * segmentLengthADB = sdaiCreateADB(sdaiREAL, &segmentLengthParameterValue);
                                        sdaiPutADBTypePath(segmentLengthADB, 1, "IFCPARAMETERVALUE");
                                        sdaiPutAttrBN(ifcCurveSegmentInstance, "SegmentLength", sdaiADB, (void*) segmentLengthADB);
                                    }
                                }
                                else {
                                    //
                                    //  SegmentStart
                                    //
                                    double  segmentStart = offset;

                                    void   * segmentStartADB = sdaiCreateADB(sdaiREAL, &segmentStart);
                                    sdaiPutADBTypePath(segmentStartADB, 1, "IFCLENGTHMEASURE");
                                    sdaiPutAttrBN(ifcCurveSegmentInstance, "SegmentStart", sdaiADB, (void*) segmentStartADB);

                                    //
                                    //  SegmentLength
                                    //
                                    void   * segmentLengthADB = sdaiCreateADB(sdaiREAL, &segmentLength);
                                    sdaiPutADBTypePath(segmentLengthADB, 1, "IFCLENGTHMEASURE");
                                    sdaiPutAttrBN(ifcCurveSegmentInstance, "SegmentLength", sdaiADB, (void*) segmentLengthADB);
                                }
                            }
                        }
                        else {
                            assert(endRadiusOfCurvature == 0.);
#ifdef _DEBUG
                            double  gradientSpiral = pStartGradient[index + 1];
#endif // _DEBUG

                            ___VECTOR3 	originSpiral = { horizontalLength, 0., 0. },
#ifdef _DEBUG
                                        refDirectionSpiral = { 1., gradientSpiral, 0. },
                                        endPoint = { 0., pStartHeight[index] - pStartHeight[index + 1], 0. },
#endif // _DEBUG
                                        correctedEndPoint;

#ifdef _DEBUG
                            ___VECTOR3 	secondPointSpiral = {
	                                            originSpiral.x + 10. * refDirectionSpiral.x,
	                                            originSpiral.y + 10. * refDirectionSpiral.y,
	                                            originSpiral.z + 10. * refDirectionSpiral.z
	                                        };
                            double	D = ___PointLineDistance(&correctedEndPoint, &endPoint, &originSpiral, &secondPointSpiral);
#endif // _DEBUG

                            double  correctedHorizontalLength = ___Vec3Distance(&originSpiral, &correctedEndPoint),
                                    correctedStartAngle = startAngle - endAngle,
                                    correctedEndAngle = 0.;

                            double  L1 = correctedStartAngle ? (correctedStartAngle / std::fabs(correctedStartAngle)) * sqrt(1. * std::fabs(correctedStartAngle) / 1.) : 0.,
                                    L2 = correctedEndAngle ? (correctedEndAngle / std::fabs(correctedEndAngle)) * sqrt(1. * std::fabs(correctedEndAngle) / 1.) : 0.;

                            double  polynomialConstants[3] = { 0., 0., 1. },
                                    x1 = ___IntegralTaylorSeriesCos(polynomialConstants, 3, L1),
                                    x2 = ___IntegralTaylorSeriesCos(polynomialConstants, 3, L2);

                            {
                                double  distance = x2 - x1;
    //                            assert(distance > 0.);

                                double  a = std::pow(distance / correctedHorizontalLength, 2);
                                polynomialConstants[2] = a;
                                L1 = correctedStartAngle ? (correctedStartAngle / std::fabs(correctedStartAngle)) * sqrt(1. * std::fabs(correctedStartAngle) / a) : 0.;
                                L2 = correctedEndAngle ? (correctedEndAngle / std::fabs(correctedEndAngle)) * sqrt(1. * std::fabs(correctedEndAngle) / a) : 0.;
    //                            assert(L1 < L2);

                                x1 = ___IntegralTaylorSeriesCos(polynomialConstants, 3, L1);
                                x2 = ___IntegralTaylorSeriesCos(polynomialConstants, 3, L2);
//                                double  dist = x2 - x1;
    //                            assert(std::fabs(dist - correctedHorizontalLength) < 0.0000001);

#ifdef _DEBUG
                                double  y1 = ___IntegralTaylorSeriesSin(polynomialConstants, 3, L1);
#endif // _DEBUG
                                assert(std::fabs(std::fabs(y1) - std::fabs(D)) < 0.0000001);

                                double  segmentLength = L2 - L1,
                                        offset = L1;

                                double  linearTerm = (correctedStartAngle / std::fabs(correctedStartAngle)) / (2. * a);

#ifdef _DEBUG
                                double  angle1 = ___AngleByAngleDeviationPolynomial(0., 0., (linearTerm) ? 1. / linearTerm : 0., 0., L1);
                                double  angle2 = ___AngleByAngleDeviationPolynomial(0., 0., (linearTerm) ? 1. / linearTerm : 0., 0., L2);
#endif // _DEBUG
                                assert(std::fabs(correctedStartAngle - angle1) < 0.000001);
                                assert(std::fabs(correctedEndAngle - angle2) < 0.000001);

                                SdaiInstance    ifcClothoidParentCurve =
                                                    ___CreateClothoidInstance(
                                                            model,
                                                            linearTerm ? segmentLength * pow(std::fabs(linearTerm), -1. / 2.) * linearTerm / std::fabs(linearTerm) : 0.,
                                                            nullptr
                                                        );
                                sdaiPutAttrBN(ifcCurveSegmentInstance, "ParentCurve", sdaiINSTANCE, (void*) ifcClothoidParentCurve);

                                if (isIFC4X3_ADD1) {
                                    if (offset >= 0. && segmentLength >= 0. &&
                                        forceUseParameterValue == false) {
                                        //
                                        //  SegmentStart
                                        //
                                        double  segmentStart = offset;

                                        void   * segmentStartADB = sdaiCreateADB(sdaiREAL, &segmentStart);
                                        sdaiPutADBTypePath(segmentStartADB, 1, "IFCNONNEGATIVELENGTHMEASURE");
                                        sdaiPutAttrBN(ifcCurveSegmentInstance, "SegmentStart", sdaiADB, (void*) segmentStartADB);

                                        //
                                        //  SegmentLength
                                        //
                                        void   * segmentLengthADB = sdaiCreateADB(sdaiREAL, &segmentLength);
                                        sdaiPutADBTypePath(segmentLengthADB, 1, "IFCNONNEGATIVELENGTHMEASURE");
                                        sdaiPutAttrBN(ifcCurveSegmentInstance, "SegmentLength", sdaiADB, (void*) segmentLengthADB);
                                    }
                                    else {
                                        //
                                        //  SegmentStart
                                        //
                                        double  segmentStartParameterValue =
                                                    ___ClothoidLengthMeasureToParameterValue(
                                                            linearTerm ? segmentLength * pow(std::fabs(linearTerm), -1. / 2.) * linearTerm / std::fabs(linearTerm) : 0.,
                                                            offset
                                                        );

                                        void   * segmentStartADB = sdaiCreateADB(sdaiREAL, &segmentStartParameterValue);
                                        sdaiPutADBTypePath(segmentStartADB, 1, "IFCPARAMETERVALUE");
                                        sdaiPutAttrBN(ifcCurveSegmentInstance, "SegmentStart", sdaiADB, (void*) segmentStartADB);

                                        //
                                        //  SegmentLength
                                        //
                                        double  segmentLengthParameterValue =
                                                    ___ClothoidLengthMeasureToParameterValue(
                                                            linearTerm ? segmentLength * pow(std::fabs(linearTerm), -1. / 2.) * linearTerm / std::fabs(linearTerm) : 0.,
                                                            segmentLength
                                                        );

                                        void   * segmentLengthADB = sdaiCreateADB(sdaiREAL, &segmentLengthParameterValue);
                                        sdaiPutADBTypePath(segmentLengthADB, 1, "IFCPARAMETERVALUE");
                                        sdaiPutAttrBN(ifcCurveSegmentInstance, "SegmentLength", sdaiADB, (void*) segmentLengthADB);
                                    }
                                }
                                else {
                                    //
                                    //  SegmentStart
                                    //
                                    double  segmentStart = offset;

                                    void   * segmentStartADB = sdaiCreateADB(sdaiREAL, &segmentStart);
                                    sdaiPutADBTypePath(segmentStartADB, 1, "IFCLENGTHMEASURE");
                                    sdaiPutAttrBN(ifcCurveSegmentInstance, "SegmentStart", sdaiADB, (void*) segmentStartADB);

                                    //
                                    //  SegmentLength
                                    //
                                    void   * segmentLengthADB = sdaiCreateADB(sdaiREAL, &segmentLength);
                                    sdaiPutADBTypePath(segmentLengthADB, 1, "IFCLENGTHMEASURE");
                                    sdaiPutAttrBN(ifcCurveSegmentInstance, "SegmentLength", sdaiADB, (void*) segmentLengthADB);
                                }
                            }
                        }
                    }
                }
                else if (___equals(predefinedType, "CONSTANTGRADIENT")) {
                    ___VECTOR2  dir = {
                                        refDirection.u,
                                        refDirection.v
                                    };

                    heightDeviation = refDirection.u ? refDirection.v * horizontalLength / refDirection.u : 0.;

                    ___startRadius = ___endRadius = 0.;

//                    if (horizontalLength) {
                    if (true) {
                        SdaiInstance    ifcLineParentCurve =
                                            ___CreateLineInstance(
                                                    model,
                                                    &dir
                                                );
                        sdaiPutAttrBN(ifcCurveSegmentInstance, "ParentCurve", sdaiINSTANCE, (void*) ifcLineParentCurve);

                        double  offset = 0.,
                                segmentLength = horizontalLength * std::sqrt(1. + startGradient__ * startGradient__);
                        assert(segmentLength >= 0.);
                        if (isIFC4X3_ADD1) {
                            if (offset >= 0. && segmentLength >= 0. &&
                                forceUseParameterValue == false) {
                                //
                                //  SegmentStart
                                //
                                double  segmentStart = offset;

                                void   * segmentStartADB = sdaiCreateADB(sdaiREAL, &segmentStart);
                                sdaiPutADBTypePath(segmentStartADB, 1, "IFCNONNEGATIVELENGTHMEASURE");
                                sdaiPutAttrBN(ifcCurveSegmentInstance, "SegmentStart", sdaiADB, (void*) segmentStartADB);

                                //
                                //  SegmentLength
                                //
                                void   * segmentLengthADB = sdaiCreateADB(sdaiREAL, &segmentLength);
                                sdaiPutADBTypePath(segmentLengthADB, 1, "IFCNONNEGATIVELENGTHMEASURE");
                                sdaiPutAttrBN(ifcCurveSegmentInstance, "SegmentLength", sdaiADB, (void*) segmentLengthADB);
                            }
                            else {
                                //
                                //  SegmentStart
                                //
                                double  segmentStartParameterValue =
                                            ___LineLengthMeasureToParameterValue(
                                                    segmentLength,
                                                    offset
                                                );

                                void   * segmentStartADB = sdaiCreateADB(sdaiREAL, &segmentStartParameterValue);
                                sdaiPutADBTypePath(segmentStartADB, 1, "IFCPARAMETERVALUE");
                                sdaiPutAttrBN(ifcCurveSegmentInstance, "SegmentStart", sdaiADB, (void*) segmentStartADB);

                                //
                                //  SegmentLength
                                //
                                double  segmentLengthParameterValue =
                                            ___LineLengthMeasureToParameterValue(
                                                    segmentLength,
                                                    segmentLength
                                                );

                                void   * segmentLengthADB = sdaiCreateADB(sdaiREAL, &segmentLengthParameterValue);
                                sdaiPutADBTypePath(segmentLengthADB, 1, "IFCPARAMETERVALUE");
                                sdaiPutAttrBN(ifcCurveSegmentInstance, "SegmentLength", sdaiADB, (void*) segmentLengthADB);
                            }
                        }
                        else {
                            //
                            //  SegmentStart
                            //
                            double  segmentStart = offset;

                            void   * segmentStartADB = sdaiCreateADB(sdaiREAL, &segmentStart);
                            sdaiPutADBTypePath(segmentStartADB, 1, "IFCLENGTHMEASURE");
                            sdaiPutAttrBN(ifcCurveSegmentInstance, "SegmentStart", sdaiADB, (void*) segmentStartADB);

                            //
                            //  SegmentLength
                            //
                            void   * segmentLengthADB = sdaiCreateADB(sdaiREAL, &segmentLength);
                            sdaiPutADBTypePath(segmentLengthADB, 1, "IFCLENGTHMEASURE");
                            sdaiPutAttrBN(ifcCurveSegmentInstance, "SegmentLength", sdaiADB, (void*) segmentLengthADB);
                        }
                    }
                    else {
                        assert(index == noSegmentInstances - 1);
                        sdaiDeleteInstance(ifcCurveSegmentInstance);
                        ifcCurveSegmentInstance = 0;
                    }
                }
                else {
                    assert(___equals(predefinedType, "PARABOLICARC"));

                    if (startGradient__ == 0. && index) {
                        startGradient__ = pEndGradient[index - 1];
                    }
                    
                    if (endGradient__ == 0. && index < noSegmentInstances - 1) {
                        endGradient__ = pStartGradient[index + 1];
                    }

                    ___startRadius = ___endRadius = 0.;

//                    double  startAngle = std::atan(startGradient__),
//                            endAngle = std::atan(endGradient__);

                    //
                    //  y = a * x^2 + b * x^1 + c * x^0
                    //    = a * x^2 + b * x   + c
                    //
                    //  direction
                    //      startAngle = 2 * a * x1 + b
                    //      endAngle = 2 * a * x2 + b
                    //      x2 - x1 = horizontalLength
                    //
                    //      horizontalLength = (endAngle - startAngle) / (2 * a)
                    //      a = (endAngle - startAngle) / (2 * horizontalLength)
                    // 
                    //      y' = 2ax + b
                    // 
                    //  start point (x1, y1) where x1 = 0
                    //      startAngle = 2 * a * x1 + b => b = startAngle
                    // 
                    //  (x, y) in x1 => (0, startHeight)
                    //       y = a * x^2 + b * x + c
                    //      startHeight = a * 0.^2 + b * 0. + c => c = startHeight
                    //

//                    double  a = (endAngle - startAngle) / (2. * horizontalLength),
                    double  a = (endGradient__ - startGradient__) / (2. * horizontalLength),
//                            b = startAngle,
                            b = startGradient__,
                            c = startHeight;

                    double  pCoefficientsX[] = { 0., 1. },
                            pCoefficientsY[] = { c, b, a };
                    SdaiInstance    ifcParabolicArcParentCurve =
                                        ___CreatePolynomialCurveInstance(
                                                model,
                                                pCoefficientsX, sizeof(pCoefficientsX) / sizeof(double),
                                                pCoefficientsY, sizeof(pCoefficientsY) / sizeof(double),
                                                nullptr, 0
                                            );
                    sdaiPutAttrBN(ifcCurveSegmentInstance, "ParentCurve", sdaiINSTANCE, (void*) ifcParabolicArcParentCurve);

                    double  offset = 0.,
                            segmentLength = horizontalLength;
                    if (isIFC4X3_ADD1) {
                        if (offset >= 0. && segmentLength >= 0. &&
                            forceUseParameterValue == false) {
                            //
                            //  SegmentStart
                            //
                            double  segmentStart = offset;

                            void   * segmentStartADB = sdaiCreateADB(sdaiREAL, &segmentStart);
                            sdaiPutADBTypePath(segmentStartADB, 1, "IFCNONNEGATIVELENGTHMEASURE");
                            sdaiPutAttrBN(ifcCurveSegmentInstance, "SegmentStart", sdaiADB, (void*) segmentStartADB);

                            //
                            //  SegmentLength
                            //
                            void   * segmentLengthADB = sdaiCreateADB(sdaiREAL, &segmentLength);
                            sdaiPutADBTypePath(segmentLengthADB, 1, "IFCNONNEGATIVELENGTHMEASURE");
                            sdaiPutAttrBN(ifcCurveSegmentInstance, "SegmentLength", sdaiADB, (void*) segmentLengthADB);
                        }
                        else {
                            //
                            //  SegmentStart
                            //
                            double  segmentStartParameterValue =
                                        ___PolynomialCurveLengthMeasureToParameterValue(
                                                pCoefficientsX, sizeof(pCoefficientsX) / sizeof(double),
                                                pCoefficientsY, sizeof(pCoefficientsY) / sizeof(double),
                                                nullptr, 0,
                                                offset
                                            );

                            void   * segmentStartADB = sdaiCreateADB(sdaiREAL, &segmentStartParameterValue);
                            sdaiPutADBTypePath(segmentStartADB, 1, "IFCPARAMETERVALUE");
                            sdaiPutAttrBN(ifcCurveSegmentInstance, "SegmentStart", sdaiADB, (void*) segmentStartADB);

                            //
                            //  SegmentLength
                            //
                            double  segmentLengthParameterValue =
                                        ___PolynomialCurveLengthMeasureToParameterValue(
                                                pCoefficientsX, sizeof(pCoefficientsX) / sizeof(double),
                                                pCoefficientsY, sizeof(pCoefficientsY) / sizeof(double),
                                                nullptr, 0,
                                                segmentLength
                                            );

                            void   * segmentLengthADB = sdaiCreateADB(sdaiREAL, &segmentLengthParameterValue);
                            sdaiPutADBTypePath(segmentLengthADB, 1, "IFCPARAMETERVALUE");
                            sdaiPutAttrBN(ifcCurveSegmentInstance, "SegmentLength", sdaiADB, (void*) segmentLengthADB);
                        }
                    }
                    else {
                        //
                        //  SegmentStart
                        //
                        double  segmentStart = offset;

                        void   * segmentStartADB = sdaiCreateADB(sdaiREAL, &segmentStart);
                        sdaiPutADBTypePath(segmentStartADB, 1, "IFCLENGTHMEASURE");
                        sdaiPutAttrBN(ifcCurveSegmentInstance, "SegmentStart", sdaiADB, (void*) segmentStartADB);

                        //
                        //  SegmentLength
                        //
                        void   * segmentLengthADB = sdaiCreateADB(sdaiREAL, &segmentLength);
                        sdaiPutADBTypePath(segmentLengthADB, 1, "IFCLENGTHMEASURE");
                        sdaiPutAttrBN(ifcCurveSegmentInstance, "SegmentLength", sdaiADB, (void*) segmentLengthADB);
                    }

                    double  radiusOfCurvature__ = 0.;
                    sdaiGetAttrBN(ifcAlignmentVerticalSegmentInstance, "RadiusOfCurvature", sdaiREAL, &radiusOfCurvature__);

                    if (radiusOfCurvature__ == 0. && a) {
                        double  radiusOfCurvature = 1. / (2. * a);
                        sdaiPutAttrBN(ifcAlignmentVerticalSegmentInstance, "RadiusOfCurvature", sdaiREAL, &radiusOfCurvature);
                    }
                }

                //
                //  Transition
                //
                SetCurveSegmentTransition4Vertical(
                        ifcCurveSegmentInstance,
                        predefinedType,
                        predefinedTypePreviousSegment,
                        ___startRadius,
                        endRadiusPreviousSegment,
                        startGradient__,
                        endGradientPreviousSegment
                    );

#ifdef _DEBUG
                if (ifcCurveSegmentInstance) {
                    ___POINT4D  startPnt = { { 0., 0., 0. }, { 0., 0., 0. }, { 0., 0., 0. } },
                                endPnt = { { 0., 0., 0. }, { 0., 0., 0. }, { 0., 0., 0. } };
                    ___GetBorderPoints(
                            ifcCurveSegmentInstance,
                            sdaiGetEntity(model, "IFCGRADIENTCURVE"),
                            &startPnt,
                            &endPnt
                        );

                    assert(startPnt.point.x == startDistAlong &&
                           startPnt.point.y == startHeight &&
                           startPnt.point.z == 0.);

                    ___VECTOR3  tangent = {
                                        1.,
                                        startGradient__,
                                        0.
                                    };
                    ___Vec3Normalize(&tangent);

                    assert(std::fabs(startPnt.tangent.x - tangent.x) < 0.0000000001 &&
                           std::fabs(startPnt.tangent.y - tangent.y) < 0.0000000001 &&
                           startPnt.tangent.z == 0.);

                    if (index) {
        //                assert(std::fabs(startPnt.point.x - previousEndPnt.point.x < 0.0000000001) &&
        //                       std::fabs(startPnt.point.y - previousEndPnt.point.y < 0.0000000001) &&
        //                       startPnt.point.z == previousEndPnt.point.z);

        //                assert(std::fabs(startPnt.tangent.x - previousEndPnt.tangent.x < 0.00001) &&
        //                       std::fabs(startPnt.tangent.y - previousEndPnt.tangent.y < 0.00001) &&
        //                       startPnt.tangent.z == previousEndPnt.tangent.z); //  */
                    }

                    previousEndPnt = endPnt;
                }
#endif // _DEBUG

                if (ifcCurveSegmentInstance)
                    sdaiAppend(aggrCurveSegment, sdaiINSTANCE, (void*) ifcCurveSegmentInstance);

                if (index == noSegmentInstances - 1) {
                    if (horizontalLength == 0.) {
                        assert(startGradient__ == endGradient__ && heightDeviation == 0.);
                        location.u = startDistAlong - startDistAlongHorizontalAlignment;
                        location.v = startHeight;
                        refDirection.u = 1.;
                        refDirection.v = startGradient__;
                        sdaiPutAttrBN(ifcGradientCurveInstance, "EndPoint", sdaiINSTANCE, (void*) ___CreateAxis2Placement2DInstance(model, &location, &refDirection));
                    }
                    else {
                        location.u = startDistAlong - startDistAlongHorizontalAlignment + horizontalLength;
                        location.v = startHeight + heightDeviation;
                        refDirection.u = 1.;
                        refDirection.v = endGradient__;
                        sdaiPutAttrBN(ifcGradientCurveInstance, "EndPoint", sdaiINSTANCE, (void*) ___CreateAxis2Placement2DInstance(model, &location, &refDirection));
                    }
                }
                else {
                    assert(horizontalLength > 0.);
                }

                mostRecentCurveSegmentInstance = ifcCurveSegmentInstance;

                predefinedTypePreviousSegment = predefinedType;
                endGradientPreviousSegment    = endGradient__;
                endRadiusPreviousSegment      = ___endRadius;
            }
        }

        delete[] pSegmentType;
        delete[] pRadiusOfCurvature;
        delete[] pStartHeight;
        delete[] pEndGradient;
        delete[] pStartGradient;

        delete[] segmentInstances;

        if (mostRecentCurveSegmentInstance) {
            ___VECTOR2  endPoint = { 0., 0. };

            ___GetEndPoint(
                    model,
                    &endPoint,
#ifdef _DEBUG
                    &mostRecentLocation,
#endif // _DEBUG
                    mostRecentCurveSegmentInstance,
                    sdaiGetInstanceType(ifcGradientCurveInstance)
                );

            ___VECTOR2  refDirection = {
                                1.,
                                mostRecentEndGradient
                            },
                        location = {
                                endPoint.u,
                                endPoint.v
                            };
            assert((mostRecentStartDistAlong + mostRecentHorizontalLength) - startDistAlongHorizontalAlignment == endPoint.u);

            if (mostRecentHorizontalLength == 0.) {
                assert(mostRecentLocation.u == endPoint.u &&
                       mostRecentLocation.v == endPoint.v);
            }

            ___Vec2Normalize(&refDirection);
            sdaiPutAttrBN(ifcGradientCurveInstance, "EndPoint", sdaiINSTANCE, (void*) ___CreateAxis2Placement2DInstance(model, &location, &refDirection));
        }

        return  ifcGradientCurveInstance;
    }
 
    return  0;
}

static  inline  SdaiInstance    ___GetAlignmentVertical(
                                        SdaiModel       model,
                                        SdaiInstance    ifcAlignmentInstance,
                                        bool            * hasIssue              = nullptr
                                    )
{
    SdaiInstance    ifcAlignmentVerticalInstance = 0;

    {
	    SdaiAggr	aggrIfcRelAggregates = nullptr;
        sdaiGetAttrBN(ifcAlignmentInstance, "IsNestedBy", sdaiAGGR, &aggrIfcRelAggregates);
        SdaiInteger noAggrIfcRelAggregates = sdaiGetMemberCount(aggrIfcRelAggregates);
        for (SdaiInteger i = 0; i < noAggrIfcRelAggregates; i++) {
            SdaiInstance    ifcRelAggregatesInstance = 0;
            sdaiGetAggrByIndex(aggrIfcRelAggregates, i, sdaiINSTANCE, &ifcRelAggregatesInstance);

            SdaiAggr    aggrIfcObjectDefinition = nullptr;
            sdaiGetAttrBN(ifcRelAggregatesInstance, "RelatedObjects", sdaiAGGR, &aggrIfcObjectDefinition);
            SdaiInteger noAggrIfcObjectDefinition = sdaiGetMemberCount(aggrIfcObjectDefinition);
            for (SdaiInteger j = 0; j < noAggrIfcObjectDefinition; j++) {
                SdaiInstance    ifcObjectDefinitionInstance = 0;
                sdaiGetAggrByIndex(aggrIfcObjectDefinition, j, sdaiINSTANCE, &ifcObjectDefinitionInstance);

                if (sdaiGetInstanceType(ifcObjectDefinitionInstance) == sdaiGetEntity(model, "IFCALIGNMENTVERTICAL")) {
                    if (ifcAlignmentVerticalInstance && hasIssue)
                        (*hasIssue) = true;

                    assert(ifcAlignmentVerticalInstance == 0);
                    ifcAlignmentVerticalInstance = ifcObjectDefinitionInstance;
                }
            }
        }
    }

    if (ifcAlignmentVerticalInstance == 0) {
        SdaiAggr    aggrIfcRelAggregates = nullptr;
        sdaiGetAttrBN(ifcAlignmentInstance, "IsDecomposedBy", sdaiAGGR, &aggrIfcRelAggregates);
        SdaiInteger noAggrIfcRelAggregates = sdaiGetMemberCount(aggrIfcRelAggregates);
        for (SdaiInteger i = 0; i < noAggrIfcRelAggregates; i++) {
            SdaiInstance    ifcRelAggregatesInstance = 0;
            sdaiGetAggrByIndex(aggrIfcRelAggregates, i, sdaiINSTANCE, &ifcRelAggregatesInstance);

            SdaiAggr    aggrIfcObjectDefinition = nullptr;
            sdaiGetAttrBN(ifcRelAggregatesInstance, "RelatedObjects", sdaiAGGR, &aggrIfcObjectDefinition);
            SdaiInteger noAggrIfcObjectDefinition = sdaiGetMemberCount(aggrIfcObjectDefinition);
            for (SdaiInteger j = 0; j < noAggrIfcObjectDefinition; j++) {
                SdaiInstance    ifcObjectDefinitionInstance = 0;
                sdaiGetAggrByIndex(aggrIfcObjectDefinition, j, sdaiINSTANCE, &ifcObjectDefinitionInstance);

                if (sdaiGetInstanceType(ifcObjectDefinitionInstance) == sdaiGetEntity(model, "IFCALIGNMENTVERTICAL")) {
                    assert(ifcAlignmentVerticalInstance == 0);
                    ifcAlignmentVerticalInstance = ifcObjectDefinitionInstance;

                    if (hasIssue)
                        (*hasIssue) = true;
                }
            }
        }
    }

    return  ifcAlignmentVerticalInstance;
}
