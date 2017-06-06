/*---------------------------------------------------------------------------*\
 *
 *  bitpit
 *
 *  Copyright (C) 2015-2017 OPTIMAD engineering Srl
 *
 *  -------------------------------------------------------------------------
 *  License
 *  This file is part of bitpit.
 *
 *  bitpit is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU Lesser General Public License v3 (LGPL)
 *  as published by the Free Software Foundation.
 *
 *  bitpit is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 *  License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with bitpit. If not, see <http://www.gnu.org/licenses/>.
 *
\*---------------------------------------------------------------------------*/

# include "CG.hpp"

# include "bitpit_operators.hpp"

# include <assert.h>
# include <lapacke.h>

namespace bitpit{

namespace CGElem{

/*!
    \ingroup CG
    \{
*/


/*!
 * Converts barycentric coordinates of a point on a segment to a flag that indicates where the point lies.
 * Flag = 0 Point lies within the segment
 * Flag = i Point coincides with the ith vertex of triangle or lies on the adjacent side of the segment
 * @param[in] lambda barycentric coordinates of point
 * @return flag
 */
int convertBarycentricToFlagSegment( std::array<double,2> const &lambda)
{

    if (lambda[0]>=1.) {
        return 1;

    } else if (lambda[1]>=1.) {
        return 2;

    } 
    
    return 0;
};

/*!
 * Converts barycentric coordinates of a point on a triangle to a flag that indicates where the point lies.
 * Flag = 0 Point lies within the triangle
 * Flag = i Point coincides with the ith vertex of triangle or lies within the area spanned by the edges incident in the ith vertex
 * Flag = -i Point lies on the edge starting from the ith vertex and connecting the following vertex in clockwise direction or in its shaddowed area
 * @param[in] lambda barycentric coordinates of point
 * @return flag
 */
int convertBarycentricToFlagTriangle( array3D const &lambda)
{

    int count = 0;
    std::array<int,2> zeros = {{0,0}};

    for( int i=0; i<3; ++i){
        if ( lambda[i] <= 0.) {
            zeros[count] = i ;
            ++count ;
        }
    }

    if( count == 1){
        int vertex0 = (zeros[0] +1) %3;
        count = -(vertex0+1) ;

    } else if (count == 2) {
        count = 3 -zeros[0] -zeros[1] +1 ;

    }

    return count;
};

/*!
 * Converts barycentric coordinates of a point on a simplex to a flag that indicates where the point lies.
 * Flag = 0 Point lies within the simplex
 * Flag = i Point coincides with the ith vertex of simplex or lies within the area spanned by the edges incident in the ith vertex
 * Flag = -i Point lies on the edge starting from the ith vertex and connecting the following vertex in clockwise direction or in its shaddowed area
 * @param[in] lambda barycentric coordinates of point
 * @return flag
 */
int convertBarycentricToFlagSimplex( std::vector<double> const &lambda)
{

    int N = lambda.size();
    int count = 0;
    std::array<int,3> positives = {{0,0,0}};

    for( int i=0; i<N; ++i){
        if ( lambda[i] > 0.) {
            positives[count] = i ;
            ++count ;
        }
    }

    if( count == 1){
        count = positives[0] +1;

    } else if (count==2) {
        int vertex0 = positives[0] +1;
        count = -(vertex0+1) ;

    } else {
        count = 0;
    }

    return count;
};

/*!
 * Reconstructs a point from barycentric coordinates of a segment
 * @param[in] Q0 first vertex of segment
 * @param[in] Q1 second vertex of segment
 * @param[in] lambda barycentric coordinates
 * @param[out] reconstructed point
 */
array3D reconstructPointFromBarycentricSegment(array3D const &Q0, array3D const &Q1, std::array<double,2> &lambda)
{
    return lambda[0]*Q0 +lambda[1]*Q1;
}

/*!
 * Reconstructs a point from barycentric coordinates of a segment
 * @param[in] Q0 first vertex of segment
 * @param[in] Q1 second vertex of segment
 * @param[in] lambda barycentric coordinates
 * @param[out] reconstructed point
 */
array3D reconstructPointFromBarycentricSegment(array3D const &Q0, array3D const &Q1, double *lambda)
{
    return lambda[0]*Q0 +lambda[1]*Q1;
}

/*!
 * Reconstructs a point from barycentric coordinates of a triangle
 * @param[in] Q0 first vertex of triangle
 * @param[in] Q1 second vertex of triangle
 * @param[in] Q2 third vertex of triangle
 * @param[in] lambda barycentric coordinates
 * @param[out] reconstructed point
 */
array3D reconstructPointFromBarycentricTriangle(array3D const &Q0, array3D const &Q1, array3D const &Q2, std::array<double,3> const &lambda)
{
    return lambda[0]*Q0 +lambda[1]*Q1 +lambda[2]*Q2;
}

/*!
 * Reconstructs a point from barycentric coordinates of a triangle
 * @param[in] Q0 first vertex of triangle
 * @param[in] Q1 second vertex of triangle
 * @param[in] Q2 third vertex of triangle
 * @param[in] lambda barycentric coordinates
 * @param[out] reconstructed point
 */
array3D reconstructPointFromBarycentricTriangle(array3D const &Q0, array3D const &Q1, array3D const &Q2, double *lambda)
{
    return lambda[0]*Q0 +lambda[1]*Q1 +lambda[2]*Q2;
}

/*!
 * Reconstructs a point from barycentric coordinates of a simplex
 * @param[in] V vertices of simplex
 * @param[in] lambda barycentric coordinates
 * @return reconstructed Point
 */
array3D reconstructPointFromBarycentricSimplex( std::vector<array3D> const &V, std::vector<double> const &lambda)
{
    array3D xP = {{0.,0.,0.}};
    int N(V.size());

    for(int i=0; i<N; ++i){
        xP += lambda[i]*V[i];
    }

    return xP;
}

/*!
 * Computes projection of point on line in 3D
 * @param[in] P point coordinates
 * @param[in] Q point on line
 * @param[in] n line direction
 * @return projection point
 */
array3D projectPointLine( array3D const &P, array3D const &Q, array3D const &n )
{
    return Q + dotProduct(P - Q, n) * n;
}

/*!
 * Computes projection of point on line in 3D
 * @param[in] P point coordinates
 * @param[in] Q point on plane
 * @param[in] n plane normal
 * @return projection point
 */
array3D projectPointPlane( array3D const &P, array3D const &Q, array3D const &n )
{
    return P - dotProduct(P - Q, n) * n;
}

/*!
 * Computes projection of point on line in 3D
 * @param[in] P point coordinates
 * @param[in] Q point on plane
 * @param[in] n plane normal
 * @return projection point
 */
array3D projectPointSegment( array3D const &P, array3D const &Q0, array3D const &Q1)
{
    std::array<double,2> lambda;
    return projectPointSegment( P, Q0, Q1, &lambda[0] );
}

/*!
 * Computes projection of point on line in 3D
 * @param[in] P point coordinates
 * @param[in] Q point on plane
 * @param[in] n plane normal
 * @param[out] lambda barycentric coordinates of projection point 
 * @return projection point
 */
array3D projectPointSegment( array3D const &P, array3D const &Q0, array3D const &Q1, std::array<double,2> &lambda )
{
    return projectPointSegment( P, Q0, Q1, &lambda[0]);
}

/*!
 * Computes projection of point on line in 3D
 * @param[in] P point coordinates
 * @param[in] Q point on plane
 * @param[in] n plane normal
 * @param[out] lambda barycentric coordinates of projection point 
 * @return projection point
 */
array3D projectPointSegment( array3D const &P, array3D const &Q0, array3D const &Q1, double *lambda )
{

    array3D n = Q1 -Q0;
    double t =  -dotProduct(n,Q0-P) / dotProduct(n,n) ;

    // Restrict projection onto the segment
    t = std::max( std::min( t, 1.), 0. ) ;

    lambda[0] = 1. - t ;
    lambda[1] = t ;

    return reconstructPointFromBarycentricSegment( Q0, Q1, lambda);
}

/*!
 * Computes projection of point on triangle
 * @param[in] P point coordinates
 * @param[in] Q0 first triangle vertex
 * @param[in] Q1 second triangle vertex
 * @param[in] Q2 third triangle vertex
 * @return coordinates of projection point
 */
array3D projectPointTriangle( array3D const &P, array3D const &Q0, array3D const &Q1, array3D const &Q2)
{
    array3D xP;
    array3D lambda;
    _projectPointsTriangle( 1, &P, Q0, Q1, Q2, &xP, lambda.data() );
    return xP;
}

/*!
 * Computes projection of point on triangle
 * @param[in] P point coordinates
 * @param[in] Q0 first triangle vertex
 * @param[in] Q1 second triangle vertex
 * @param[in] Q2 third triangle vertex
 * @param[out] lambda barycentric coordinates of projection point
 * @return coordinates of projection point
 */
array3D projectPointTriangle( array3D const &P, array3D const &Q0, array3D const &Q1, array3D const &Q2, array3D &lambda)
{
    array3D xP;
    _projectPointsTriangle( 1, &P, Q0, Q1, Q2, &xP, lambda.data() );

    return xP ;
}

/*!
 * Restricts a point given in barycentric coordinates on the triangle
 * @param[in] Q0 first triangle vertex
 * @param[in] Q1 second triangle vertex
 * @param[in] Q2 third triangle vertex
 * @param[in,out] lambda barycentric coordinates before and after restriction
 * @return coordinates of restricted point
 */
array3D restrictPointTriangle( array3D const &Q0, array3D const &Q1, array3D const &Q2, array3D &lambda)
{
    return restrictPointTriangle( Q0, Q1, Q2, &lambda[0] );
}

/*!
 * Restricts a point given in barycentric coordinates on the triangle
 * @param[in] Q0 first triangle vertex
 * @param[in] Q1 second triangle vertex
 * @param[in] Q2 third triangle vertex
 * @param[in,out] lambda barycentric coordinates before and after restriction
 * @return coordinates of restricted point
 */
array3D restrictPointTriangle( array3D const &Q0, array3D const &Q1, array3D const &Q2, double *lambda)
{

    std::array<const array3D*,3> r = {{&Q0, &Q1, &Q2}} ;

    int count = 0;
    std::array<int,2> negatives = {{ 0, 0 }};

    for( int i=0; i<3; ++i){
        if( lambda[i] < 0){
            negatives[count] = i ;
            ++count ;
        }
    }

    if( count == 0){
        return reconstructPointFromBarycentricTriangle( Q0, Q1, Q2, lambda );

    } else if( count == 1){
        std::array<double,2>   lambdaLocal ;
        int vertex0 = (negatives[0] +1) %3;
        int vertex1 = (vertex0     +1) %3;
        array3D P = reconstructPointFromBarycentricTriangle( Q0, Q1, Q2, lambda ); 
        array3D xP = projectPointSegment(P, *r[vertex0], *r[vertex1], lambdaLocal);
        lambda[negatives[0]] = 0. ;
        lambda[vertex0] = lambdaLocal[0] ;
        lambda[vertex1] = lambdaLocal[1] ;
        return xP;

    } else {
        int vertex0 = 3 -negatives[0] -negatives[1] ;
        lambda[0] = 0.;
        lambda[1] = 0.;
        lambda[2] = 0.;
        lambda[vertex0] = 1. ;
        return *r[vertex0] ;

    }

    BITPIT_UNREACHABLE("CANNOT REACH");

}

/*!
 * Computes distances of point cloud to triangle
 * @param[in] cloud point cloud coordinates
 * @param[in] Q1 first triangle vertex
 * @param[in] Q2 second triangle vertex
 * @param[in] Q3 third triangle vertex
 * @param[inout] xPExt pointer to std::vector to be filled with the projection point; 
 * @param[inout] lambdas pointer to sd::vector to be filled with barycentric coordinates of projection points
 * @return distances
 */
std::vector<array3D> projectCloudTriangle( std::vector<array3D> const &cloud, array3D const &Q0, array3D const &Q1, array3D const &Q2, std::vector<array3D> &lambda )
{
   
    int cloudCount(cloud.size()); 

    std::vector<array3D> xP(cloudCount);
    xP.shrink_to_fit();

    lambda.resize(cloudCount);
    lambda.shrink_to_fit();

    _projectPointsTriangle( cloudCount, cloud.data(), Q0, Q1, Q2, xP.data(), &lambda[0][0]);

    return xP ;

};

/*!
 * Computes distances of point cloud to triangle
 * @param[in] cloud point cloud coordinates
 * @param[in] Q1 first triangle vertex
 * @param[in] Q2 second triangle vertex
 * @param[in] Q3 third triangle vertex
 * @param[out] proj pointer to the projection point; 
 * @param[out] lambdas pointer to barycentric coordinates of projection points
 * @return distances
 */
void _projectPointsTriangle( int nPoints, array3D const *point, array3D const &Q0, array3D const &Q1, array3D const &Q2, array3D *proj, double *lambda )
{

    array3D s0 = Q1-Q0;
    array3D s1 = Q2-Q0;

    double A[4] = { dotProduct(s0,s0), 0, dotProduct(s0,s1), dotProduct(s1,s1) }   ; 
    double *B = new double [2*nPoints] ;

    for( int i=0; i<nPoints; ++i){
        array3D rP = *point -Q0 ;
        B[2*i]   = dotProduct(s0,rP) ; 
        B[2*i+1] = dotProduct(s1,rP) ; 
        ++point;
    }

    int info =  LAPACKE_dposv( LAPACK_COL_MAJOR, 'U', 2, nPoints, A, 2, B, 2 ) ;
    assert( info == 0 );
    BITPIT_UNUSED( info ) ;

    for( int i=0; i<nPoints; ++i){

        double *b = &B[2*i] ;

        lambda[0] = 1. -b[0] -b[1] ;
        lambda[1] = b[0] ;
        lambda[2] = b[1] ;

        *proj = restrictPointTriangle( Q0, Q1, Q2, lambda);
        lambda +=3 ;
        proj += 1;
    }

    delete [] B ;
};

/*!
 * Project a point cloud on a plane described by a triangle
 * @param[in] cloud point cloud coordinates
 * @param[in] Q1 first triangle vertex
 * @param[in] Q2 second triangle vertex
 * @param[in] Q3 third triangle vertex
 * @param[out] proj pointer to the projection point; 
 * @param[out] lambdas pointer to barycentric coordinates of projection points
 * @return distances
 */
void _projectPointsPlane( int nPoints, array3D const *point, array3D const &Q0, array3D const &Q1, array3D const &Q2, array3D *proj, double *lambda )
{

    array3D s0 = Q1-Q0;
    array3D s1 = Q2-Q0;

    double A[4] = { dotProduct(s0,s0), 0, dotProduct(s0,s1), dotProduct(s1,s1) }   ; 
    double *B = new double [2*nPoints] ;

    for( int i=0; i<nPoints; ++i){
        array3D rP = *point -Q0 ;
        B[2*i]   = dotProduct(s0,rP) ; 
        B[2*i+1] = dotProduct(s1,rP) ; 
        ++point;
    }

    int info =  LAPACKE_dposv( LAPACK_COL_MAJOR, 'U', 2, nPoints, A, 2, B, 2 ) ;
    assert( info == 0 );
    BITPIT_UNUSED( info ) ;

    for( int i=0; i<nPoints; ++i){

        double *b = &B[2*i] ;

        lambda[0] = 1. -b[0] -b[1] ;
        lambda[1] = b[0] ;
        lambda[2] = b[1] ;

        *proj = reconstructPointFromBarycentricTriangle( Q0, Q1, Q2, lambda);
        lambda +=3 ;
        proj += 1;
    }

    delete [] B ;
};


/*!
 * Computes projection of point onto a generic simplex
 * @param[in] P point coordinates
 * @param[in] V simplex vertices coordinates
 * @return coordinates of projection point
 */
array3D projectPointSimplex( array3D const &P, std::vector<array3D> const &V)
{
    std::vector<double> lambda;
    return projectPointSimplex( P, V, lambda);
}

/*!
 * Computes projection of point onto a generic simplex
 * @param[in] P point coordinates
 * @param[in] V simplex vertices coordinates
 * @param[out] lambda baycentric coordinates of projection point
 * @return coordinates of projection point
 */
array3D projectPointSimplex( array3D const &P, std::vector<array3D> const &V, std::vector<double> &lambda)
{

    array3D xP;

    int vertexCount(V.size());
    lambda.resize(vertexCount);
    lambda.shrink_to_fit();

    if (vertexCount == 2) { //segment
        xP =  projectPointSegment(P, V[0], V[1], lambda.data());

    } else if (vertexCount == 3) { //triangle
        _projectPointsTriangle(1, &P, V[0], V[1], V[2], &xP, &lambda[0] );

    } else { //generic convec polygon
        double distance, minDistance(std::numeric_limits<double>::max());
        int minTriangle;
        array3D localLambda, minLambda;

        // Compute the distance from each triangle in the simplex
        int triangleCount = vertexCount - 2;

        int vertex0 = 0;
        int vertex1 = 1;
        int vertex2 = 2;
        for (int triangle=0; triangle < triangleCount; ++triangle) {

            distance = distancePointTriangle(P, V[vertex0], V[vertex1], V[vertex2], localLambda);

            if (distance <= minDistance) {

                minDistance = distance;

                minLambda = localLambda;
                minTriangle = triangle;
            }

            vertex1++;
            vertex2++;

        } //next triangle

        lambda.assign(vertexCount,0.);
        vertex0 = 0;
        vertex1 = 1+minTriangle;
        vertex2 = 2+minTriangle;
        lambda[vertex0] = minLambda[0];
        lambda[vertex1] = minLambda[1];
        lambda[vertex2] = minLambda[2];

        xP = reconstructPointFromBarycentricTriangle( V[vertex0], V[vertex1], V[vertex2], minLambda);

    }

    return xP;

};

/*!
 * Computes projection point on semi-infinite cone surface
 * @param[in] point point coordinates
 * @param[in] apex cone apex
 * @param[in] axis cone axis
 * @param[in] alpha cone half angle
 * @return projection point
 */
array3D projectPointCone( array3D const &point, array3D const &apex, array3D const &axis, double const &alpha){


    if( alpha <= M_PI/2. ) { //accute cone angle

        array3D versor = point-apex;
        versor /= norm2(versor);

        double cosPointAxis = dotProduct(versor,axis);
        double cosCriticalAngle = cos(alpha+M_PI/2.);

        if( cosPointAxis <= cosCriticalAngle ){ //point projects on cone apex
            return apex;

        } else { // point projects on cone surface

            array3D planeNormal = crossProduct(axis,versor);
            planeNormal /= norm2(planeNormal);

            array3D direction = rotateVector(axis,planeNormal,alpha);

            return projectPointLine(point,apex,direction);

        } 

    } else { // abtuse cone angle -> project on complement
        return projectPointCone( point, apex, -1.*axis, M_PI-alpha);

    }

}

/*!
 * Computes distance point to line in 3D
 * @param[in] P point coordinates
 * @param[in] Q point on line
 * @param[in] n line direction
 * @param[out] xP closest point on line
 * @return distance
 */
double distancePointLine(
        std::array< double, 3 > const &P,
        std::array< double, 3 > const &Q,
        std::array< double, 3 > const &n,
        std::array< double, 3 > &xP
        ) {

    xP = projectPointLine(P,Q,n);
    return norm2( P-xP);
};

/*!
 * Computes distance point to plane
 * @param[in] P point coordinates
 * @param[in] Q point on plane
 * @param[in] n plane normal
 * @param[out] xP closest point on line
 * @return distance
 */
double distancePointPlane(
        std::array< double, 3 > const &P,
        std::array< double, 3 > const &Q,
        std::array< double, 3 > const &n,
        std::array< double, 3 > &xP
        ) {

    xP = projectPointPlane(P,Q,n);
    return norm2(P-xP);
}

/*!
 * Computes distance point to segment in 3D using a projection method
 * @param[in] P point coordinates
 * @param[in] Q1 segment starting point
 * @param[in] Q2 segment ending point
 * @param[out] xP closest point on line
 * @param[out] flag point mapping onto segment vertices (flag = 1, 2), or onto segment interior (flag = 0)
 * @return distance
 */
double distancePointSegment(
        std::array< double, 3 > const &P,
        std::array< double, 3 > const &Q1,
        std::array< double, 3 > const &Q2,
        std::array< double, 3 >       &xP,
        int                      &flag
        ) {

    std::array<double,2> lambda;

    double distance =  distancePointSegment(P, Q1, Q2, lambda); 
    xP = reconstructPointFromBarycentricSegment(Q1,Q2,lambda);
    flag = convertBarycentricToFlagSegment(lambda);
    return distance;
};

/*!
 * Computes distance point to segment in 3D using barycentric coordinates
 * @param[in] P point coordinates
 * @param[in] Q1 segment starting point
 * @param[in] Q2 segment ending point
 * @param[out] xP closest point on line
 * @param[out] lambda barycentric coordinates
 * @return distance
 */
double distancePointSegment(
        std::array< double, 3 > const &P,
        std::array< double, 3 > const &Q1,
        std::array< double, 3 > const &Q2,
        std::array< double, 3 >       &xP,
        std::array< double, 2 >       &lambda,
        int                      &flag
        ) {

    xP = projectPointSegment( P, Q1, Q2, lambda);
    flag = convertBarycentricToFlagSegment(lambda);

    return norm2(P-xP); 
};

/*!
 * Computes distance point to segment in 3D using barycentric coordinates
 * @param[in] P point coordinates
 * @param[in] Q0 segment starting point
 * @param[in] Q1 segment ending point
 * @return distance
 */
double distancePointSegment( array3D const &P, array3D const &Q0, array3D const &Q1)
{ 
    array3D xP = projectPointSegment( P, Q0, Q1);
    return norm2(P-xP); 
};

/*!
 * Computes distance point to segment in 3D using barycentric coordinates
 * @param[in] P point coordinates
 * @param[in] Q1 segment starting point
 * @param[in] Q2 segment ending point
 * @param[out] xP closest point on line
 * @param[out] lambda barycentric coordinates
 * @return distance
 */
double distancePointSegment( array3D const &P, array3D const &Q0, array3D const &Q1, std::array<double,2> &lambda)
{
    array3D xP = projectPointSegment( P, Q0, Q1, lambda);
    return norm2(P-xP); 
};

/*!
 * Computes distance point to triangle
 * @param[in] P point coordinates
 * @param[in] Q0 first triangle vertex
 * @param[in] Q1 second triangle vertex
 * @param[in] Q2 third triangle vertex
 * @param[out] xP closest point on triangle
 * @param[out] flag point projecting onto triangle's interior (flag = 0), triangle's vertices (flag = 1, 2, 3) or triangle's edges (flag = -1, -2, -3)
 * @return distance
 */
double distancePointTriangle(
        std::array< double, 3 > const &P,
        std::array< double, 3 > const &Q0,
        std::array< double, 3 > const &Q1,
        std::array< double, 3 > const &Q2,
        std::array< double, 3 >       &xP,
        int                      &flag
        ) {

    std::array<double,3> lambda;
    double distance =  distancePointTriangle( P, Q0, Q1, Q2, lambda);
    flag = convertBarycentricToFlagTriangle(lambda);
    xP = reconstructPointFromBarycentricTriangle( Q0, Q1, Q2, lambda);
    return distance;

};

/*!
 * Computes distance point to triangle
 * @param[in] P point coordinates
 * @param[in] Q0 first triangle vertex
 * @param[in] Q1 second triangle vertex
 * @param[in] Q2 third triangle vertex
 * @param[out] xP closest point on triangle
 * @param[out] lambda barycentric coordinates of projection point
 * @param[out] flag point projecting onto triangle's interior (flag = 0), triangle's vertices (flag = 1, 2, 3) or triangle's edges (flag = -1, -2, -3)
 * @return distance
 */
double distancePointTriangle(
        std::array< double, 3 > const &P,
        std::array< double, 3 > const &Q0,
        std::array< double, 3 > const &Q1,
        std::array< double, 3 > const &Q2,
        std::array< double, 3 >       &xP,
        std::array< double, 3 >       &lambda,
        int                           &flag
        ) {

    xP = projectPointTriangle(P, Q0, Q1, Q2, lambda);
    flag = convertBarycentricToFlagTriangle(lambda);

    return norm2(P-xP);

};

/*!
 * Computes distance point to triangle
 * @param[in] P point coordinates
 * @param[in] Q0 first triangle vertex
 * @param[in] Q1 second triangle vertex
 * @param[in] Q2 third triangle vertex
 * @return distance
 */
double distancePointTriangle( array3D const &P, array3D const &Q0, array3D const &Q1, array3D const &Q2)
{
    array3D xP = projectPointTriangle(P, Q0, Q1, Q2);
    return norm2(P-xP);

};

/*!
 * Computes distance point to triangle
 * @param[in] P point coordinates
 * @param[in] Q0 first triangle vertex
 * @param[in] Q1 second triangle vertex
 * @param[in] Q2 third triangle vertex
 * @param[out] lambda barycentric coordinates of projection point
 * @return distance
 */
double distancePointTriangle( array3D const &P, array3D const &Q0, array3D const &Q1, array3D const &Q2, array3D &lambda)
{
    array3D xP = projectPointTriangle(P, Q0, Q1, Q2, lambda);
    return norm2(P-xP);
};

/*!
 * Computes distance point to semi-infinite cone surface
 * @param[in] point point coordinates
 * @param[in] apex cone apex
 * @param[in] axis cone axis
 * @param[in] alpha cone half angle
 * @return distance
 */
double distancePointCone( array3D const &point, array3D const &apex, array3D const &axis, double const &alpha){

    array3D xP = projectPointCone( point, apex, axis, alpha);
    return norm2(point-xP);

}

/*!
 * Computes distances of point cloud to triangle
 * @param[in] P point cloud coordinates
 * @param[in] Q1 first triangle vertex
 * @param[in] Q2 second triangle vertex
 * @param[in] Q3 third triangle vertex
 * @param[out] xP closest points on triangle
 * @param[out] flag point projecting onto triangle's interior (flag = 0), triangle's vertices (flag = 1, 2, 3) or triangle's edges (flag = -1, -2, -3)
 * @return distance
 */
std::vector<double> distanceCloudTriangle(
        std::vector< std::array< double, 3 > >    const &P,
        std::array< double, 3 >              const &Q1,
        std::array< double, 3 >              const &Q2,
        std::array< double, 3 >              const &Q3,
        std::vector< std::array< double, 3 > >    &xP,
        std::vector< int >                   &flag
        ) {

    int N( P.size() );
    std::vector<array3D> lambda;
    std::vector<double> d = distanceCloudTriangle( P, Q1, Q2, Q3, lambda );

    flag.resize(N);
    flag.shrink_to_fit();
    std::vector<int>::iterator flagItr = flag.begin();

    xP.resize(N);
    xP.shrink_to_fit();
    std::vector<array3D>::iterator xPItr = xP.begin();

    for( auto const &l : lambda){
        *xPItr   = reconstructPointFromBarycentricTriangle( Q1, Q2, Q3, l);
        *flagItr = convertBarycentricToFlagTriangle(l);

        ++xPItr;
        ++flagItr;
    }

    return d;

};

/*!
 * Computes distances of point cloud to triangle
 * @param[in] cloud point cloud coordinates
 * @param[in] Q1 first triangle vertex
 * @param[in] Q2 second triangle vertex
 * @param[in] Q3 third triangle vertex
 * @param[inout] xPExt pointer to std::vector to be filled with the projection point; 
 * @param[inout] lambdas pointer to sd::vector to be filled with barycentric coordinates of projection points
 * @return distances
 */
std::vector<double> distanceCloudTriangle(
        std::vector<array3D> const &cloud,
        array3D              const &Q0,
        array3D              const &Q1,
        array3D              const &Q2,
        std::vector<array3D>* const xPExt,
        std::vector<array3D>* const lambdaExt
        ) {

    int N(cloud.size());

    std::vector<array3D> xP, lambda;

    std::vector<array3D> *xPPtr = ( xPExt == nullptr) ? &xP : xPExt;
    std::vector<array3D> *lambdaPtr = ( lambdaExt == nullptr) ? &lambda : lambdaExt;
    
    xPPtr->resize(N);
    lambdaPtr->resize(N);

    _projectPointsTriangle( N, cloud.data(), Q0, Q1, Q2, &(xPPtr->at(0)), &(*lambdaPtr)[0][0] );

    std::vector<array3D>::iterator projection = xPPtr->begin();

    std::vector<double> d(N);
    std::vector<double>::iterator distance = d.begin();

    for( const auto &point : cloud){
        *distance = norm2( point - *projection);

        ++projection;
        ++distance;
    }

    return d;

};

/*!
 * Computes distances of point cloud to triangle
 * @param[in] cloud point cloud coordinates
 * @param[in] Q1 first triangle vertex
 * @param[in] Q2 second triangle vertex
 * @param[in] Q3 third triangle vertex
 * @return distances
 */
std::vector<double> distanceCloudTriangle( std::vector<array3D> const &cloud, array3D const &Q0, array3D const &Q1, array3D const &Q2)
{ 

    std::vector<array3D> lambda(cloud.size());
    return distanceCloudTriangle( cloud, Q0, Q1, Q2, lambda);
};

/*!
 * Computes distances of point cloud to triangle
 * @param[in] cloud point cloud coordinates
 * @param[in] Q1 first triangle vertex
 * @param[in] Q2 second triangle vertex
 * @param[in] Q3 third triangle vertex
 * @param[out] lambda barycentric coordinates of projection points
 * @return distances
 */
std::vector<double> distanceCloudTriangle( std::vector<array3D> const &cloud, array3D const &Q0, array3D const &Q1, array3D const &Q2, std::vector<array3D> &lambda )
{ 
    int N(cloud.size());

    lambda.resize(N);

    std::vector<array3D> xP(N);
    _projectPointsTriangle( N, cloud.data(), Q0, Q1, Q2, xP.data(), &lambda[0][0]);

    std::vector<double> d(N);
    std::vector<double>::iterator distance = d.begin();

    std::vector<array3D>::iterator projection = xP.begin();
    for( const auto &point : cloud){
        *distance = norm2( point - *projection);

        ++projection;
        ++distance;
    }

    return d;
};

/*!
 * Computes distances of point to generic simplex
 * @param[in] P point coordinates
 * @param[in] V simplex vertices coordinates
 * @param[out] xP closest points on simplex
 * @param[out] flag point projecting onto simplex's interior (flag = 0), simplex's vertices (flag = 1, 2, ...) or triangle's edges (flag = -1, -2, -...)
 * @return distance
 */
double distancePointSimplex( array3D const &P, std::vector<array3D> const &V, array3D &xP, int &flag)
{

    std::vector<double> lambda;
    double distance = distancePointSimplex( P, V, lambda);
    xP = reconstructPointFromBarycentricSimplex( V, lambda );
    flag = convertBarycentricToFlagSimplex( lambda );

    return distance; 

};

/*!
 * Computes distances of point to generic simplex
 * @param[in] P point coordinates
 * @param[in] V simplex vertices coordinates
 * @return distance
 */
double distancePointSimplex( array3D const &P, std::vector<array3D> const &V)
{
    std::vector<double> lambda(V.size());
    return distancePointSimplex( P, V, lambda);

};

/*!
 * Computes distances of point to generic simplex
 * @param[in] P point coordinates
 * @param[in] V simplex vertices coordinates
 * @param[out] lambda barycentric coordinates
 * @return distance
 */
double distancePointSimplex( array3D const &P, std::vector<array3D> const &V,std::vector<double> &lambda)
{
    array3D xP = projectPointSimplex( P, V, lambda);
    return norm2(P-xP); 

};

/*!
 * Computes distances of point cloud to generic simplex
 * @param[in] P point cloud coordinates
 * @param[in] V simplex vertices coordinates
 * @param[out] xP closest points on simplex
 * @param[out] flag point projecting onto simplex's interior (flag = 0), simplex's vertices (flag = 1, 2, ...) or triangle's edges (flag = -1, -2, -...)
 * @return distance
 */
std::vector<double> distanceCloudSimplex( std::vector<array3D> const &cloud, std::vector<array3D> const &V, std::vector<array3D> &xP, std::vector<int> &flag)
{

    std::vector<std::vector<double>> lambda;
    std::vector<double> d = distanceCloudSimplex( cloud, V, lambda);

    int cloudCount( cloud.size() );

    xP.resize(cloudCount);
    xP.shrink_to_fit();
    std::vector<array3D>::iterator xPItr = xP.begin();

    flag.resize(cloudCount);
    flag.shrink_to_fit();
    std::vector<int>::iterator flagItr = flag.begin();

    for( const auto &l : lambda){
        *flagItr = convertBarycentricToFlagSimplex( l );
        *xPItr = reconstructPointFromBarycentricSimplex( V, l ); 

        ++xPItr;
        ++flagItr;
    }

    return d; 
};

/*!
 * Computes distances of point cloud to generic simplex
 * @param[in] P point cloud coordinates
 * @param[in] V simplex vertices coordinates
 * @return distance
 */
std::vector<double> distanceCloudSimplex( std::vector<array3D> const &P, std::vector<array3D> const &V)
{
    int cloudCount(P.size()), vertexCount(V.size());


    if (vertexCount == 2) { //Segment
        std::vector<double> d(cloudCount) ;

        for( int i=0; i<cloudCount; ++i ){
            d[i] = distancePointSegment(P[i], V[0], V[1]);
        }

        return d;

    } else if (vertexCount == 3) {  // Triangle 
        return distanceCloudTriangle(P, V[0], V[1], V[2]);

    } else { // Generic convex polygon 

        std::vector<double> d(cloudCount,std::numeric_limits<double>::max()) ;

        int triangleCount = vertexCount - 2;
        int vertex0 = 0;
        int vertex1 = 1;
        int vertex2 = 2;

        for (int triangle=0; triangle < triangleCount; ++triangle) { // foreach triangle
            std::vector<double> dT = distanceCloudTriangle(P, V[vertex0], V[vertex1], V[vertex2]);

            d = min(d,dT) ;

            ++vertex1;
            ++vertex2;

        }

        return d; 
    }

    BITPIT_UNREACHABLE("CANNOT REACH");


};

/*!
 * Computes distances of point cloud to generic simplex
 * @param[in] cloud point cloud coordinates
 * @param[in] V simplex vertices coordinates
 * @param[out] lambda barycentric coordinates of projectio points
 * @return distance
 */
std::vector<double> distanceCloudSimplex( std::vector<array3D> const &cloud, std::vector<array3D> const &V, std::vector<std::vector<double>> &lambda)
{
    int cloudCount(cloud.size()), vertexCount(V.size());

    lambda.resize(cloudCount, std::vector<double>(vertexCount,0) );
    lambda.shrink_to_fit();

    if (vertexCount == 2) { //Segment
        std::vector<double> d(cloudCount) ;
        d.shrink_to_fit();

        std::vector<double>::iterator dItr = d.begin();
        std::vector<std::vector<double>>::iterator lambdaItr = lambda.begin();

        array3D proj;

        for( auto const &point : cloud ){
            proj = projectPointSegment(point, V[0], V[1], lambdaItr->data());
            *dItr = norm2( point-proj);
            ++dItr;
            ++lambdaItr;
        }

        return d;

    } else if (vertexCount == 3) {  // Triangle 
        std::vector<array3D> lambdaTemp(cloudCount) ;
        auto lambdaItr = lambda.begin();
        std::vector<double> d =  distanceCloudTriangle(cloud, V[0], V[1], V[2], lambdaTemp);
        for(auto const &l : lambdaTemp){
            std::copy( l.begin(), l.end(), lambdaItr->begin());
            ++lambdaItr;
        }

    } else { // Generic convex polygon 

        std::vector<double> d(cloudCount,std::numeric_limits<double>::max()) ;
        std::vector<double> dTemp(cloudCount) ;
        std::vector<array3D> lambdaTemp(cloudCount) ;

        int triangleCount = vertexCount - 2;
        int vertex0 = 0;
        int vertex1 = 1;
        int vertex2 = 2;

        for (int triangle=0; triangle < triangleCount; ++triangle) { // foreach triangle
            dTemp = distanceCloudTriangle(cloud, V[vertex0], V[vertex1], V[vertex2], lambdaTemp);

            for(int i=0; i< cloudCount; ++i){
                if( dTemp[i] < d[i]){
                    d[i] = dTemp[i];
                    std::copy( lambdaTemp[i].begin(), lambdaTemp[i].end(), lambda[i].begin());
                }
            }

            ++vertex1;
            ++vertex2;

        }

        return d; 
    }

    BITPIT_UNREACHABLE("CANNOT REACH");
};

/*!
 * Computes distance between two lines in 3D
 * @param[in] n0 direction of first line
 * @param[in] P0 point on first line
 * @param[in] n1 direction of second line
 * @param[in] P1 point on second line
 * @return distance
 */
double distanceLineLine( array3D const &P0, array3D const &n0, array3D const &P1, array3D const &n1)
{
    array3D xP0, xP1;
    return distanceLineLine( P0, n0, P1, n1, xP0, xP1);
}

/*!
 * Computes distance between two lines in 3D
 * @param[in] n0 direction of first line
 * @param[in] P0 point on first line
 * @param[in] n1 direction of second line
 * @param[in] P1 point on second line
 * @param[out] xP0 projection of line1 on line0
 * @param[out] xP1 projection of line0 on line1
 * @return distance
 */
double distanceLineLine( array3D const &P0, array3D const &n0, array3D const &P1, array3D const &n1, array3D &xP0, array3D &xP1)
{
    double n01 = dotProduct(n0,n1);
    double det = 1. - n01*n01;

    // check if lines are parallel 
    if( std::abs(det) < 1.e-12){
        double distance = distancePointLine(P0, P1, n1, xP1);
        xP0 = projectPointLine(xP1, P0, n0);
        return distance;
    }


    std::array<double,3> dP = P1-P0;
    double rhs0 =  dotProduct(dP,n0);
    double rhs1 = -dotProduct(dP,n1);

    double det0 = rhs0 +rhs1*n01;
    double det1 = rhs1 +rhs0*n01;

    double s0 = det0/det;
    double s1 = det1/det;

    xP0 = P0 +s0*n0;
    xP1 = P1 +s1*n1;

    return norm2( xP0 - xP1); 
}

/*!
 * Computes intersection between two lines in 3D
 * @param[in] n1 direction of first line
 * @param[in] P1 point on first line
 * @param[in] n2 direction of second line
 * @param[in] P2 point on second line
 * @param[out] P intersection point if intersect, else unaltered
 * @return if intersect
 */
bool intersectLineLine(
        std::array<double, 3> const &P1,
        std::array<double, 3> const &n1,
        std::array<double, 3> const &P2,
        std::array<double, 3> const &n2,
        std::array<double, 3>       &P
        ) {

    double tol = 1.e-12;
    array3D xP1, xP2;
    if( distanceLineLine(P1,n1,P2,n2,xP1,xP2) < tol){
        P = xP1;
        return true;
    }

    return false;

};

/*!
 * Computes intersection between two segments in 3D
 * @param[in] P1 start point of first segment
 * @param[in] P2 end point of first segment
 * @param[in] Q1 start point of second segment
 * @param[in] Q2 end point of second segment
 * @param[out] x intersection point if intersect, else unaltered
 * @return if intersect
 */
bool intersectSegmentSegment(
        std::array<double, 3> const &P1,
        std::array<double, 3> const &P2,
        std::array<double, 3> const &Q1,
        std::array<double, 3> const &Q2,
        std::array<double, 3>       &x
        ) {

    // Parameters
    double const                    abs_tol = 1.0e-14;

    // Local variables
    double                          lP, lQ, lxP, lxQ;
    std::array<double, 3>                nP, nQ;

    // Counters
    // none

    // ========================================================================== //
    // COMPUTE INTERSECTION POINT BETWEEN SUPPORTING LINES                        //
    // ========================================================================== //
    nP = P2 - P1;
    lP = norm2(nP);
    nP = nP/lP;
    nQ = Q2 - Q1;
    lQ = norm2(nQ);
    nQ = nQ/lQ;

    // ========================================================================== //
    // CHECK IF INTERSECTION POINTS LIES INSIDE EACH SEGMENT                      //
    // ========================================================================== //
    if ( intersectLineLine(P1, nP, Q1, nQ, x) ) {
        lxP = dotProduct(x - P1, nP);
        lxQ = dotProduct(x - Q1, nQ);

        if ( lxP <= lP + abs_tol && lxP >= -abs_tol && lxQ  <= lQ + abs_tol && lxQ  >= -abs_tol )  {

            return(true);
        }

    }

    return (false);


};

/*!
 * Computes intersection between line and plane
 * @param[in] P1 point on line
 * @param[in] n1 direction of line
 * @param[in] P2 point on plane
 * @param[in] n2 normal to plane
 * @param[out] P intersection point if intersect, else unaltered
 * @return if intersect
 */
bool intersectLinePlane(
        std::array<double, 3> const &P1,
        std::array<double, 3> const &n1,
        std::array<double, 3> const &P2,
        std::array<double, 3> const &n2,
        std::array<double, 3>       &P
        ) {

    // Parameters
    double const                tol = 1.0e-14;

    // Local variables
    double                      s, xi;

    // ========================================================================== //
    // CHECK DEGENERATE CASES                                                     //
    // ========================================================================== //
    s = dotProduct(n1, n2);
    if (std::abs(s) < tol) { return(false); }

    // ========================================================================== //
    // FIND INTERSECTION POINTS                                                   //
    // ========================================================================== //
    xi = -dotProduct(P1 - P2, n2)/s;
    P = P1 + xi * n1;

    return(true); 
};

/*!
 * Computes intersection between segment and plane
 * @param[in] Q1 start point of segment
 * @param[in] Q2 end point of segment
 * @param[in] P2 point on plane
 * @param[in] n2 normal to plane
 * @param[out] P intersection point if intersect, else unaltered
 * @return if intersect
 */
bool intersectSegmentPlane(
        std::array<double, 3> const &Q1,
        std::array<double, 3> const &Q2,
        std::array<double, 3> const &P2,
        std::array<double, 3> const &n2,
        std::array<double, 3>       &P
        ) {

    std::array<double, 3> n, xP;

    n = Q2 - Q1;
    n /= norm2(n);

    if ( intersectLinePlane(Q1, n, P2, n2, xP) && intersectPointSegment(xP, Q1, Q2) ) {
        P = xP;
        return true;
    }

    return false ; 
};

/*!
 * Computes intersection between two planes
 * @param[in] P1 point on first plane
 * @param[in] n1 normal to first plane
 * @param[in] P2 point on second plane
 * @param[in] n2 normal to second plane
 * @param[out] Pl point on intersection line
 * @param[out] nl direction of intersection line
 * @return if intersect
 */
bool intersectPlanePlane(
        std::array<double, 3> const &P1,
        std::array<double, 3> const &n1,
        std::array<double, 3> const &P2,
        std::array<double, 3> const &n2,
        std::array<double, 3>       &Pl,
        std::array<double, 3>       &nl
        ) {

    double const tol = 1.0e-14;
    double n12 = dotProduct(n1, n2);
    double detCB = 1.0-n12*n12;

    // check degenerate condition
    if( std::abs(detCB) <= tol) { 
        return false ; 
    }

    nl = crossProduct(n1,n2);
    nl /= norm2(nl);

    // if planes intersect, determine the closest point
    // to P1 and P2 as anchor point. The augmented functional
    // I = 0.5*[ (Pl-P1)^2 + (Pl-P2)^2] + lambda1[ n1.(Pl-P1) ] +lambda2[ n2.(Pl-P2) ]
    // where lambda1 and lambda2 are Lagrange multipliers.
    // The optimality conditions I,Pl I,lambda1 I,lambda2 are 
    // solved using the Schur complment

    std::array<double,3>  dP = P2-P1;
    std::array<double,2>  rhs = {{ dotProduct(n1,dP) , -dotProduct(n2,dP) }};

    double det1 = rhs[0] - n12*rhs[1];
    double det2 = rhs[1] - n12*rhs[0];
    double lambda1 = det1 /detCB;
    double lambda2 = det2 /detCB;

    Pl = P1 +P2 -lambda1*n1 -lambda2*n2;
    Pl *= 0.5;

    return true ; 

};

/*!
 * Computes intersection between triangle and a line
 * @param[in] P point on plane
 * @param[in] n normal to plane
 * @param[in] A first vertex of triangle
 * @param[in] B second vertex of triangle
 * @param[in] C third vertex of triangle
 * @param[out] Q intersection point
 * @return if intersect
 */
bool intersectLineTriangle(
        std::array<double, 3> const &P,
        std::array<double, 3> const &n,
        std::array<double, 3> const &A,
        std::array<double, 3> const &B,
        std::array<double, 3> const &C,
        std::array<double, 3>       &Q
        ) {

    std::array<double, 3> nT, xP;

    nT  = crossProduct(B - A, C - A);
    nT /= norm2(nT);

    if ( !intersectLinePlane(P, n, A, nT, xP) && !intersectPointTriangle(xP, A, B, C) ) { 
        return(false); 
    };

    Q = xP;
    return true ; 
};

/*!
 * Computes intersection between triangle and a segment
 * @param[in] P0 start point of segment
 * @param[in] P1 end point of segment
 * @param[in] A first vertex of triangle
 * @param[in] B second vertex of triangle
 * @param[in] C third vertex of triangle
 * @param[out] Q intersection point
 * @return if intersect
 */
bool intersectSegmentTriangle(
        std::array<double, 3> const &P0,
        std::array<double, 3> const &P1,
        std::array<double, 3> const &A,
        std::array<double, 3> const &B,
        std::array<double, 3> const &C,
        std::array<double, 3>       &Q
        ) {

    std::array<double, 3> n, xP;

    n  = P1 - P0;
    n /= norm2(n);

    if ( !intersectLineTriangle(P0, n, A, B, C, xP) && !intersectPointSegment(xP, P0, P1) ) { 
        return false ; 
    }

    Q = xP;
    return true ; 

};

/*!
 * Computes intersection between triangle and a segment
 * @param[in] P point on line
 * @param[in] n direction of line
 * @param[in] V simplex vertices coordinates
 * @param[out] Q intersection point
 * @return if intersect
 */
bool intersectLineSimplex(
        std::array<double, 3> const          &P,
        std::array<double, 3> const          &n,
        std::vector<std::array<double, 3> > const &V,
        std::array<double, 3>                &Q
        ) {

    int nTriangles = V.size() -2;

    int vertex0 = 0;
    int vertex1 = 1;
    int vertex2 = 2;
    for( int i=0; i< nTriangles; ++i){

        if( intersectLineTriangle(P, n, V[vertex0], V[vertex1], V[vertex2], Q) ) { 
            return true ; 
        }

        ++vertex1;
        ++vertex2;
    }

    return false ; 
};

/*!
 * Computes intersection between triangle and a segment
 * @param[in] P0 start point of segment
 * @param[in] P1 end point of segment
 * @param[in] V simplex vertices coordinates
 * @param[out] Q intersection point
 * @return if intersect
 */
bool intersectSegmentSimplex(
        std::array<double, 3> const          &P0,
        std::array<double, 3> const          &P1,
        std::vector<std::array<double, 3> > const &V,
        std::array<double, 3>                &Q 
        ) {

    int nTriangles = V.size() -2;

    int vertex0 = 0;
    int vertex1 = 1;
    int vertex2 = 2;
    for( int i=0; i< nTriangles; ++i){

        if( intersectSegmentTriangle(P0, P1, V[vertex0], V[vertex1], V[vertex2], Q) ) { 
            return true ; 
        }

        ++vertex1;
        ++vertex2;
    }

    return false;
}


/*!
 * Computes intersection between two axis aligned bounding boxes
 * @param[in] A1 min point of first box
 * @param[in] A2 max point of first box
 * @param[in] B1 min point of second box
 * @param[in] B2 max point of second box
 * @param[in] dim number of dimension to be checked
 * @return if intersect
 */
bool intersectBoxBox(
        std::array<double, 3> const              &A1,
        std::array<double, 3> const              &A2,
        std::array<double, 3> const              &B1,
        std::array<double, 3> const              &B2,
        int                                 dim
        ){

    int     d;

    for( d=0; d<dim; ++d){

        if( B1[d] > A2[d] || B2[d] < A1[d] ){
            return false;
        };

    };

    return true;

};

/*!
 * Computes intersection between two axis aligned bounding boxes
 * @param[in] A1 min point of first box
 * @param[in] A2 max point of first box
 * @param[in] B1 min point of second box
 * @param[in] B2 max point of second box
 * @param[out] I1 min point of intersection box
 * @param[out] I2 max point of intersection box
 * @param[in] dim number of dimension to be checked
 * @return if intersect
 */
bool intersectBoxBox(
        std::array<double, 3> const              &A1,
        std::array<double, 3> const              &A2,
        std::array<double, 3> const              &B1,
        std::array<double, 3> const              &B2,
        std::array<double, 3>                    &I1,
        std::array<double, 3>                    &I2,
        int                                 dim
        ){

    int     d;

    for( d=0; d<dim; ++d){

        if( B1[d] > A2[d] || B2[d] < A1[d] ){
            return false;
        }

        else{
            I1[d] = std::max( A1[d], B1[d] ) ;
            I2[d] = std::min( A2[d], B2[d] ) ;
        };

    };

    return true;

};

/*!
 * Computes intersection between an axis aligned bounding box and a triangle
 * @param[in] A1 min point of first box
 * @param[in] A2 max point of first box
 * @param[in] V1 first vertex of triangle
 * @param[in] V2 first vertex of triangle
 * @param[in] V3 first vertex of triangle
 * @return if intersect
 */
bool intersectBoxTriangle(
        std::array<double, 3> const              &A1, 
        std::array<double, 3> const              &A2,
        std::array<double, 3> const              &V1,
        std::array<double, 3> const              &V2,
        std::array<double, 3> const              &V3
        ){

    std::array<double,3>     B1, B2, E1, E2, P ;

    P.fill(0.0) ;

    //Check if Triangle Boundig Box and Box overlap -> necessary condition
    computeAABBTriangle( V1, V2, V3, B1, B2) ;

    if( !intersectBoxBox( A1, A2, B1, B2) ) { return(false); }

    //Check if Triangle Box edges intersection 
    for( int i=0; i<12; ++i){
        edgeOfBox( i, A1, A2, E1, E2) ;
        if( intersectSegmentTriangle(E1,E2,V1,V2,V3,P)) { return(true); }
    };

    return (false);

};


/*!
 * Computes intersection between an axis aligned bounding box and a triangle
 * @param[in] A1 min point of first box
 * @param[in] A2 max point of first box
 * @param[in] V1 first vertex of triangle
 * @param[in] V2 first vertex of triangle
 * @param[in] V3 first vertex of triangle
 * @param[out] P intersection points between triangle and box edges
 * @return if intersect
 */
bool intersectBoxTriangle(
        std::array<double, 3> const              &A1,
        std::array<double, 3> const              &A2,
        std::array<double, 3> const              &V1,
        std::array<double, 3> const              &V2,
        std::array<double, 3> const              &V3,
        std::vector< std::array<double, 3> >          &P
        ){

    bool                intersect(false) ;
    std::array<double,3>     B1, B2, E1, E2, p ;

    P.clear() ;
    P.reserve(3) ;

    //Check if Triangle Boundig Box and Box overlap -> necessary condition
    computeAABBTriangle( V1, V2, V3, B1, B2) ;

    if( !intersectBoxBox( A1, A2, B1, B2) ) { return(false); }

    //Check if Triangle Box edges intersection 
    for( int i=0; i<12; ++i){
        edgeOfBox( i, A1, A2, E1, E2) ;
        if( intersectSegmentTriangle(E1,E2,V1,V2,V3,p)) {
            P.push_back( p );
            intersect = true ;
        } ;
    };

    return intersect;

};

/*!
 * Computes intersection between an axis aligned bounding box and a segment
 * @param[in] V1 start point of segment
 * @param[in] V2 end point of segment
 * @param[in] A1 min point of box
 * @param[in] A2 max point of box
 * @param[in] dim number of dimension to be checked
 * @return if intersect
 */
bool intersectSegmentBox(
        std::array<double, 3> const              &V1,
        std::array<double, 3> const              &V2,
        std::array<double, 3> const              &A1,
        std::array<double, 3> const              &A2,
        int                                 dim
        ){

    int                 i ;
    std::array<double,3>     B1, B2, p ;

    //Check if Triangle Boundig Box and Box overlap -> necessary condition
    computeAABBSegment( V1, V2, B1, B2) ;
    if( !intersectBoxBox( A1, A2, B1, B2, dim) ) { return(false); }

    //Check if Triangle Box edges intersection 
    if( dim == 2){

        std::array<double,3> E1, E2 ;

        for( i=0; i<4; ++i){
            edgeOfBox( i, A1, A2, E1, E2) ;

            if( intersectSegmentSegment(E1,E2,V1,V2,p)) { return(true); } ;

        }


    } else if (dim == 3){ //3D

        std::vector< std::array<double,3> > E ;
        E.resize(4) ;

        for( i=0; i<12; ++i){
            faceOfBox( i, A1, A2, E[0], E[1], E[2], E[3]) ;

            if( intersectSegmentSimplex(V1,V2,E,p) ) { return(true); } ;

        }

    }

    return (false);

};

/*!
 * Computes intersection between an axis aligned bounding box and a segment
 * @param[in] V1 start point of segment
 * @param[in] V2 end point of segment
 * @param[in] A1 min point of box
 * @param[in] A2 max point of box
 * @param[out] P intersection point segment and box faces
 * @param[in] dim number of dimension to be checked
 * @return if intersect
 */
bool intersectSegmentBox(
        std::array<double, 3> const              &V1,
        std::array<double, 3> const              &V2,
        std::array<double, 3> const              &A1,
        std::array<double, 3> const              &A2,
        std::vector< std::array<double, 3> >          &P,
        int                                 dim
        ){

    bool                intersect(false) ;
    int                 i ;
    std::array<double,3>     p, B1, B2 ;

    P.clear() ;

    //Check if segment Boundig Box and Box overlap -> necessary condition
    computeAABBSegment( V1, V2, B1, B2) ;
    if( !intersectBoxBox( A1, A2, B1, B2, dim) ) { return(false); }

    P.reserve(2) ;

    if( dim == 2){ //check if box edge and segment intersect


        std::array<double,3> E1, E2 ;

        for( i=0; i<4; ++i){
            edgeOfBox( i, A1, A2, E1, E2) ;

            if( intersectSegmentSegment(E1,E2,V1,V2,p)) {
                intersect = true ;
                P.push_back(p) ;
            } ;

        };

        return( intersect ) ;

    } else if( dim==3 ) { //3D check if box face and segment intersect


        std::vector< std::array<double,3> > E ;
        E.resize(4) ;

        for( i=0; i<12; ++i){
            faceOfBox( i, A1, A2, E[0], E[1], E[2], E[3]) ;

            if( intersectSegmentSimplex(V1,V2,E,p) ) {
                intersect = true ;
                P.push_back(p) ;
            } ;

        };

        return intersect ;

    };


};


/*!
 * Computes intersection between an axis aligned bounding box and a simplex
 * @param[in] A1 min point of first box
 * @param[in] A2 max point of first box
 * @param[in] VS simplex vertices coordinates
 * @param[in] dim number of dimension to be checked
 * @return if intersect
 */
bool intersectBoxSimplex(
        std::array<double, 3> const              &A1,
        std::array<double, 3> const              &A2,
        std::vector< std::array<double, 3> > const    &VS,
        int                                 dim
        ){

    int                 i, j, n, m, p ;
    std::array<double,3>     B1, B2 ;

    //Check if Triangle Boundig Box and Box overlap -> necessary condition
    computeAABBSimplex( VS, B1, B2) ;

    if( !intersectBoxBox( A1, A2, B1, B2, dim) ) { return(false); }

    n = VS.size() ;

    if( n == 2){
        return( intersectSegmentBox( VS[0], VS[1], A1, A2, dim ) ) ;
    }

    else if( n == 3){
        return(  intersectBoxTriangle( A1, A2, VS[0], VS[1], VS[2] ) ) ;
    }

    else{

        // Generic convex polygon ----------------------------------------------- //

        // Compute the distance from each triangle in the simplex
        p = n - 2;
        m = 0;
        j = 1;
        while (m < p) {
            i = j;
            j = i+1;

            if( intersectBoxTriangle( A1, A2, VS[0], VS[i], VS[j] ) ){
                return (true);
            };

            m++;
        } //next i

    };

    return (false);

};


/*!
 * Computes intersection between an axis aligned bounding box and a simplex
 * @param[in] A1 min point of first box
 * @param[in] A2 max point of first box
 * @param[in] VS simplex vertices coordinates
 * @param[out] P intersection points simplex box edges
 * @param[in] dim number of dimension to be checked
 * @return if intersect
 */
bool intersectBoxSimplex(
        std::array<double, 3> const              &A1,
        std::array<double, 3> const              &A2,
        std::vector< std::array<double, 3> > const    &VS,
        std::vector< std::array<double, 3> >          &P,
        int                                 dim
        ){

    int                         n  ;

    P.clear() ;

    { //Check if Triangle Boundig Box and Box overlap -> necessary condition

        std::array<double,3>             B1, B2 ;
        computeAABBSimplex( VS, B1, B2) ;

        if( !intersectBoxBox( A1, A2, B1, B2, dim) ) { return(false); }
    }


    n = VS.size() ;
    if( n == 2){ //segment
        return( intersectSegmentBox( VS[0], VS[1], A1, A2, P, dim ) ) ;
    }

    else if( n == 3){ //triangle
        return(  intersectBoxTriangle( VS[0], VS[1], VS[2], A1, A2, P ) ) ;
    }

    else{  //generic convex polygon split into triangles

        bool                        intersect(false) ;
        int                         i, j, m, p ;
        std::vector< std::array<double, 3> >  partial ;

        p = n - 2;
        m = 0;
        j = 1;
        while (m < p) {
            i = j;
            j = i+1;

            if( intersectBoxTriangle( A1, A2, VS[0], VS[i], VS[j], partial ) ){
                intersect = true ;
                P.insert( P.begin(), partial.begin(), partial.end() ) ;
            };

            m++;
        } //next i

        return intersect ;

    };


};



//to levelset // -------------------------------------------------------------------------- //
//to levelset bool intersectLineSurface(
//to levelset         std::array<double,3>  const  &x1,
//to levelset         std::array<double,3>  const  &n1,
//to levelset         std::array<double,3>  const  &x2,
//to levelset         std::array<double,3>  const  &n2,
//to levelset         std::array<double,3>  const  &xL,
//to levelset         std::array<double,3>  const  &nL,
//to levelset         std::array<double,3>         &xp,
//to levelset         std::array<double,3>         &np
//to levelset         ) {
//to levelset 
//to levelset     // ========================================================================== //
//to levelset     //                                                                            //
//to levelset     // Reconstruct Surface given by two points and their normals and computes     //
//to levelset     // intersection of reconstruction with a line given by point and versor     ' //
//to levelset     // ========================================================================== //
//to levelset     // INPUT                                                                      //
//to levelset     // ========================================================================== //
//to levelset     // ========================================================================== //
//to levelset     // OUTPUT                                                                     //
//to levelset     // ========================================================================== //
//to levelset     // - none                                                                     //
//to levelset     // ========================================================================== //
//to levelset 
//to levelset     // ========================================================================== //
//to levelset     // VARIABLES DECLARATION                                                      //
//to levelset     // ========================================================================== //
//to levelset 
//to levelset     double                  w1(0), w2(0), w(0) ;
//to levelset     std::array<double,3>         c1, c2 ;
//to levelset     bool                    s1, s2 ;
//to levelset 
//to levelset     s1      =   intersectLinePlane( xL, nL, x1, n1, c1) ;
//to levelset     s2      =   intersectLinePlane( xL, nL, x2, n2, c2) ;
//to levelset 
//to levelset     if( s1 && s2) {
//to levelset         w1      =   norm2( c2-x2) ;
//to levelset         w2      =   norm2( c1-x1) ;
//to levelset         w       =   w1+w2 ;
//to levelset 
//to levelset         w1      =   w1 /w ;
//to levelset         w2      =   w2 /w ;
//to levelset 
//to levelset         xp      =   w1*c1 +w2*c2 ;
//to levelset         np      =   w1*n1 +w2*n2 ;
//to levelset 
//to levelset         np      =   np /norm2(np) ;
//to levelset 
//to levelset         return (true) ;
//to levelset     }
//to levelset 
//to levelset     else if( s1 ){
//to levelset         xp      =   c1 ;
//to levelset         np      =   n1 ;
//to levelset 
//to levelset         return(true) ;
//to levelset     }
//to levelset 
//to levelset     else if( s2 ){
//to levelset         xp      =   c2 ;
//to levelset         np      =   n2 ;
//to levelset 
//to levelset         return(true) ;
//to levelset     }
//to levelset 
//to levelset     return (false); 
//to levelset 
//to levelset };

/*!
 * checks if points lies on segment in 3D
 * @param[in] P point coordinates
 * @param[in] P1 start point of segment
 * @param[in] P2 end point of segment
 * @return if point lies on segment
 */
bool intersectPointSegment(
        std::array< double, 3> const &P,
        std::array< double, 3> const &P1,
        std::array< double, 3> const &P2
        ) {

    double const        tol = 1.0e-14;

    bool                check;
    std::array<double, 3>    n1, n2;
    double              d1, d2;

    check = (norm2(P - P2) <= tol);
    if (!check) {
        n1 = P1 - P2;
        d1 = norm2(n1);
        n1 = n1/d1;
        n2 = P - P2;
        d2 = norm2(n2);
        n2 = n2/d2;
        check = ((dotProduct(n1, n2) >= 1.0 - tol) && (d2 <= d1));
    }

    return(check); };

/*!
 * checks if points lies on triangle
 * @param[in] P point coordinates
 * @param[in] A first vertex of triangle
 * @param[in] B second vertex of triangle
 * @param[in] C third vertex of triangle
 * @return if point lies on triangle
 */
bool intersectPointTriangle( array3D const &P, array3D const &A, array3D const &B, array3D const &C)
{

    array3D xP, lambda;

    _projectPointsPlane( 1, &P, A, B, C, &xP, &lambda[0]);

    for( const auto &l : lambda){
        if(l<0.) return false;
    }

    return true;
};

/*!
 * checks if points lies within axis aligned box
 * @param[in] P point coordinates
 * @param[in] B1 min coodinates of box
 * @param[in] B2 max coodinates of box
 * @param[in] dim number of dimensions to be checked
 * @return if point in box
 */
bool intersectPointBox(
        std::array<double, 3> const              &P,
        std::array<double, 3> const              &B1,
        std::array<double, 3> const              &B2,
        int                                 dim
        ){


    int     d;

    for( d=0; d<dim; ++d){

        if( P[d]< B1[d] || P[d] > B2[d] ){
            return false;
        };
    };

    return true ;
};

/*!
 * computes axis aligned boundig box of a segment
 * @param[in] A start point of segment
 * @param[in] B end point of segment
 * @param[out] P0 min point of bounding box
 * @param[out] P1 max point of bounding box
 */
void computeAABBSegment(
        std::array<double, 3> const &A,
        std::array<double, 3> const &B,
        std::array<double, 3>       &P0,
        std::array<double, 3>       &P1
        ) {

    int     i;

    P0 = A ;
    P1 = A ;

    for(i=0; i<3; ++i){
        P0[i] = std::min( P0[i], B[i] ) ;
        P1[i] = std::max( P1[i], B[i] ) ;
    };

    return;

};

/*!
 * computes axis aligned boundig box of a triangle
 * @param[in] A first vertex of triangle
 * @param[in] B second vertex of triangle
 * @param[in] C third vertex of triangle
 * @param[out] P0 min point of bounding box
 * @param[out] P1 max point of bounding box
 */
void computeAABBTriangle(
        std::array<double, 3> const &A,
        std::array<double, 3> const &B,
        std::array<double, 3> const &C,
        std::array<double, 3>       &P0,
        std::array<double, 3>       &P1
        ) {

    int     i;

    P0 = A ;
    P1 = A ;

    for(i=0; i<3; ++i){
        P0[i] = std::min( P0[i], B[i] ) ;
        P0[i] = std::min( P0[i], C[i] ) ;

        P1[i] = std::max( P1[i], B[i] ) ;
        P1[i] = std::max( P1[i], C[i] ) ;
    };

    return;

};

/*!
 * computes axis aligned boundig box of a simplex
 * @param[in] VS simplex vertices coordinates
 * @param[out] P0 min point of bounding box
 * @param[out] P1 max point of bounding box
 */
void computeAABBSimplex(
        std::vector< std::array<double, 3> > const &VS,
        std::array<double, 3>       &P0,
        std::array<double, 3>       &P1
        ) {

    int     i, j, n(VS.size());

    P0 = VS[0] ;
    P1 = VS[0] ;

    for( j=1; j<n; ++j){
        for(i=0; i<3; ++i){
            P0[i] = std::min( P0[i], VS[j][i] ) ;
            P1[i] = std::max( P1[i], VS[j][i] ) ;
        };
    };

    return;

};

/*!
 * computes the union of two axis aligned bounding boxes
 * @param[in] A0 min point of first bounding box
 * @param[in] A1 max point of first bounding box
 * @param[in] B0 min point of second bounding box
 * @param[in] B1 max point of second bounding box
 * @param[out] C0 min point of union bounding box
 * @param[out] C1 max point of union bounding box
 */
void unionAABB(
        std::array<double,3>  const              & A0,
        std::array<double,3>  const              & A1,
        std::array<double,3>  const              & B0,
        std::array<double,3>  const              & B1,
        std::array<double,3>                     & C0,
        std::array<double,3>                     & C1
        ){


    for( int i=0; i<3; ++i){
        C0[i] = std::min( A0[i], B0[i]) ;
        C1[i] = std::max( A1[i], B1[i]) ;
    }


    return;
};

/*!
 * computes the union of several axis aligned bounding boxes
 * @param[in] A0 min points of bounding boxes
 * @param[in] A1 max points of bounding boxes
 * @param[out] C0 min point of union bounding box
 * @param[out] C1 max point of union bounding box
 */
void unionAABB(
        std::vector<std::array<double,3> >  const     & A0,
        std::vector<std::array<double,3> >  const     & A1,
        std::array<double,3>                     & C0,
        std::array<double,3>                     & C1
        ){


    int     i, n( std::min(A0.size(), A1.size() ) ) ;

    if( n > 0 ){
        C0 =  A0[0] ;
        C1 =  A1[0] ;

        for( i=1; i<n; ++i){
            unionAABB( A0[i], A1[i], C0, C1, C0, C1);
        };
    };


    return;
};

/*!
 * computes the intersection of two axis aligned bounding boxes
 * @param[in] A0 min point of first bounding box
 * @param[in] A1 max point of first bounding box
 * @param[in] B0 min point of second bounding box
 * @param[in] B1 max point of second bounding box
 * @param[out] C0 min point of union bounding box
 * @param[out] C1 max point of union bounding box
 */
void intersectionAABB(
        std::array<double,3>  const              & A0,
        std::array<double,3>  const              & A1,
        std::array<double,3>  const              & B0,
        std::array<double,3>  const              & B1,
        std::array<double,3>                     & C0,
        std::array<double,3>                     & C1
        ){

    intersectBoxBox( A0, A1, B0, B1, C0, C1 );
    return;
};

/*!
 * computes the relative complement two axis aligned bounding boxes
 * @param[in] A0 min point of first bounding box
 * @param[in] A1 max point of first bounding box
 * @param[in] B0 min point of second bounding box
 * @param[in] B1 max point of second bounding box
 * @param[out] C0 min point of union bounding box
 * @param[out] C1 max point of union bounding box
 */
void subtractionAABB(
        std::array<double,3>  const              & A0,
        std::array<double,3>  const              & A1,
        std::array<double,3>  const              & B0,
        std::array<double,3>  const              & B1,
        std::array<double,3>                     & C0,
        std::array<double,3>                     & C1
        ){

    // X direction
    if( B0[1]<=A0[1] && B0[2]<=A0[2] && B1[1]>=A1[1] && B1[2]>=A1[2] ){
        C0[0] = ( B0[0]<=A0[0] && B1[0]>=A0[0] ) ? B1[0] : A0[0] ;
        C1[0] = ( B0[0]<=A1[0] && B1[0]>=A1[0] ) ? B0[0] : A1[0] ;
    }

    // Y direction
    if( B0[0]<=A0[0] && B0[2]<=A0[2] && B1[0]>=A1[0] && B1[2]>=A1[2] ){
        C0[1] = ( B0[1]<=A0[1] && B1[1]>=A0[1] ) ? B1[1] : A0[2] ;
        C1[1] = ( B0[1]<=A1[1] && B1[1]>=A1[1] ) ? B0[1] : A1[2] ;
    }

    // Z direction
    if( B0[0]<=A0[0] && B0[1]<=A0[1] && B1[0]>=A1[0] && B1[1]>=A1[1] ){
        C0[2] = ( B0[2]<=A0[2] && B1[2]>=A0[2] ) ? B1[2] : A0[2] ;
        C1[2] = ( B0[2]<=A1[2] && B1[2]>=A1[2] ) ? B0[2] : A1[2] ;
    }


    return;
};


/*!
 * computes the face coordiantes of a box
 * @param[in] i face index
 * @param[in] A0 min point of bounding box
 * @param[in] A1 max point of bounding box
 * @param[out] P0 first vertex of face
 * @param[out] P1 first vertex of face
 * @param[out] P2 first vertex of face
 * @param[out] P3 first vertex of face
 */
void faceOfBox(
        int              const &i,
        std::array<double, 3> const &A0,
        std::array<double, 3> const &A1,
        std::array<double, 3>       &P0,
        std::array<double, 3>       &P1,
        std::array<double, 3>       &P2,
        std::array<double, 3>       &P3
        ) {

    int     v0, v1, v2, v3;

    v0 = boxFaceVertexConnectivity[i][0] ;
    v1 = boxFaceVertexConnectivity[i][1] ;
    v2 = boxFaceVertexConnectivity[i][2] ;
    v3 = boxFaceVertexConnectivity[i][3] ;

    vertexOfBox(v0, A0, A1, P0 ) ;
    vertexOfBox(v1, A0, A1, P1 ) ;
    vertexOfBox(v2, A0, A1, P2 ) ;
    vertexOfBox(v3, A0, A1, P3 ) ;

    return;

};


/*!
 * computes the edge coordiantes of a box
 * @param[in] i edge index
 * @param[in] A0 min point of bounding box
 * @param[in] A1 max point of bounding box
 * @param[out] P0 first vertex of edge
 * @param[out] P1 first vertex of edge
 */
void edgeOfBox(
        int              const &i,
        std::array<double, 3> const &A0,
        std::array<double, 3> const &A1,
        std::array<double, 3>       &P0,
        std::array<double, 3>       &P1
        ) {

    int     v0, v1;

    v0 = boxEdgeVertexConnectivity[i][0] ;
    v1 = boxEdgeVertexConnectivity[i][1] ;

    vertexOfBox(v0, A0, A1, P0 ) ;
    vertexOfBox(v1, A0, A1, P1 ) ;

    return;

};

/*!
 * computes the vertex coordiantes of a box
 * @param[in] i vertex index
 * @param[in] A0 min point of bounding box
 * @param[in] A1 max point of bounding box
 * @param[out] P vertex coordinates
 */
void vertexOfBox(
        int              const &i,
        std::array<double, 3> const &A0,
        std::array<double, 3> const &A1,
        std::array<double, 3>       &P
        ) {

    switch(i){

        case 0:
            P[0] = A0[0] ;
            P[1] = A0[1] ;
            P[2] = A0[2] ;
            break;

        case 1:
            P[0] = A1[0] ;
            P[1] = A0[1] ;
            P[2] = A0[2] ;
            break;

        case 2:
            P[0] = A0[0] ;
            P[1] = A1[1] ;
            P[2] = A0[2] ;
            break;

        case 3:
            P[0] = A1[0] ;
            P[1] = A1[1] ;
            P[2] = A0[2] ;
            break;

        case 4:
            P[0] = A0[0] ;
            P[1] = A0[1] ;
            P[2] = A1[2] ;
            break;

        case 5:
            P[0] = A1[0] ;
            P[1] = A0[1] ;
            P[2] = A1[2] ;
            break;

        case 6:
            P[0] = A0[0] ;
            P[1] = A1[1] ;
            P[2] = A1[2] ;
            break;

        case 7:
            P[0] = A1[0] ;
            P[1] = A1[1] ;
            P[2] = A1[2] ;
            break;

    };


    return;

};

/*!
 * rotates a vector in 3D using Rodrigues' formula.
 * @param[in] vector vector to be rotated
 * @param[in] axis rotation axis
 * @param[in] theta rotation angle
 * @return rotated vector
 */
array3D rotateVector( array3D const &vector, array3D const &axis, double theta){

    array3D rotated;
    double cosTheta = cos(theta);

    rotated  = cosTheta * vector;
    rotated += sin(theta) * crossProduct(axis,vector);
    rotated += (1.0 - cosTheta) * dotProduct(axis,vector) * axis;

    return rotated; 
};

}

}
