/***************************************************************************
 *   Copyright (C) 2005-2011 by the resistivity.net development team       *
 *   Carsten Rücker carsten@resistivity.net                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "triangleWrapper.h"

#include "mesh.h"
#include "meshentities.h"
#include "node.h"

#ifdef HAVE_LIBTRIANGLE

extern "C"{

#define REAL double
#define VOID void
#define TRILIBRARY
#define ANSI_DECLARATORS

struct triangulateio {
  REAL *pointlist;                                               /* In / out */
  REAL *pointattributelist;                                      /* In / out */
  int *pointmarkerlist;                                          /* In / out */
  int numberofpoints;                                            /* In / out */
  int numberofpointattributes;                                   /* In / out */

  int *trianglelist;                                             /* In / out */
  REAL *triangleattributelist;                                   /* In / out */
  REAL *trianglearealist;                                         /* In only */
  int *neighborlist;                                             /* Out only */
  int numberoftriangles;                                         /* In / out */
  int numberofcorners;                                           /* In / out */
  int numberoftriangleattributes;                                /* In / out */

  int *segmentlist;                                              /* In / out */
  int *segmentmarkerlist;                                        /* In / out */
  int numberofsegments;                                          /* In / out */

  REAL *holelist;                        /* In / pointer to array copied out */
  int numberofholes;                                      /* In / copied out */

  REAL *regionlist;                      /* In / pointer to array copied out */
  int numberofregions;                                    /* In / copied out */

  int *edgelist;                                                 /* Out only */
  int *edgemarkerlist;            /* Not used with Voronoi diagram; out only */
  REAL *normlist;                /* Used only with Voronoi diagram; out only */
  int numberofedges;                                             /* Out only */
};

void triangulate(char *, struct triangulateio *, struct triangulateio *, struct triangulateio *);

}
#endif // #ifdef HAVE_LIBTRIANGLE

namespace GIMLI{

TriangleWrapper::TriangleWrapper( const Mesh & inMesh ) : inMesh_( & inMesh ){
    init_();
}

TriangleWrapper::TriangleWrapper( const Mesh & inMesh, Mesh & outMesh, const std::string & triSwitches )
    : inMesh_( & inMesh ) {
    init_();
    switches_ = triSwitches;
    generate( outMesh );
}

TriangleWrapper::~TriangleWrapper(){
    freeMemory_();
#ifdef HAVE_LIBTRIANGLE
    if ( mesh_input_ )          delete mesh_input_;
    if ( mesh_output_ )         delete mesh_output_;
    if ( mesh_voronoi_output_ ) delete mesh_voronoi_output_;
#else
    std::cerr << WHERE_AM_I << " Triangle not installed" << std::endl;
#endif
}

void TriangleWrapper::init_( ){
    switches_ = "-pze";
#ifdef HAVE_LIBTRIANGLE
    mesh_input_           = new struct triangulateio;
    mesh_output_          = new struct triangulateio;
    mesh_voronoi_output_  = new struct triangulateio;
    allocateOutMemory_();
#else
    std::cerr << WHERE_AM_I << " Triangle not installed" << std::endl;
#endif
}

void TriangleWrapper::generate( Mesh & mesh ){
#ifdef HAVE_LIBTRIANGLE

    if ( inMesh_->nodeCount() < 3 ){
        throwError( 1, WHERE_AM_I + " input mesh must have at least 3 nodes " );
    }
    if ( mesh_output_->pointlist ) {
        freeMemory_();
        allocateOutMemory_();
    }
    transformMeshToTriangle_( *inMesh_, * mesh_input_ );
    triangulate( (char *)switches_.c_str(), mesh_input_, mesh_output_, mesh_voronoi_output_ );
    transformTriangleToMesh_( * mesh_output_, mesh );
#else
    std::cerr << WHERE_AM_I << " Triangle not installed" << std::endl;
#endif
}

void TriangleWrapper::allocateOutMemory_(){
#ifdef HAVE_LIBTRIANGLE

    mesh_output_->pointlist             = (REAL *) NULL;
    mesh_output_->pointattributelist    = (REAL *) NULL;
    mesh_output_->pointmarkerlist       = (int *) NULL;

    mesh_output_->trianglelist          = (int *) NULL;
    mesh_output_->triangleattributelist = (REAL *) NULL;

    mesh_output_->segmentlist           = (int *) NULL;
    mesh_output_->segmentmarkerlist     = (int *) NULL;

    mesh_output_->edgelist              = (int *) NULL;
    mesh_output_->edgemarkerlist        = (int *) NULL;
    mesh_output_->normlist              = (REAL *) NULL;
#else
    std::cerr << WHERE_AM_I << " Triangle not installed" << std::endl;
#endif
}

void TriangleWrapper::freeMemory_(){
#ifdef HAVE_LIBTRIANGLE
    if ( mesh_output_->pointlist != (REAL*)NULL )           free( mesh_output_->pointlist );
    if ( mesh_output_->pointattributelist != (REAL*)NULL )  free( mesh_output_->pointattributelist );
    if ( mesh_output_->pointmarkerlist != (int*)NULL  )     free( mesh_output_->pointmarkerlist ) ;

    if ( mesh_output_->trianglelist != (int*)NULL )         free( mesh_output_->trianglelist );
    if ( mesh_output_->triangleattributelist != (REAL*)NULL ) free( mesh_output_->triangleattributelist );

    if ( mesh_output_->segmentlist != (int*)NULL )          free( mesh_output_->segmentlist );
    if ( mesh_output_->segmentmarkerlist != (int*)NULL )    free( mesh_output_->segmentmarkerlist );

    if ( mesh_output_->edgelist != (int*)NULL )             free( mesh_output_->edgelist );
    if ( mesh_output_->edgemarkerlist != (int*)NULL )       free( mesh_output_->edgemarkerlist );
    if ( mesh_output_->normlist != (REAL *) NULL )          free( mesh_output_->normlist );
        
    delete [] mesh_input_->pointlist;
    delete [] mesh_input_->pointmarkerlist;
    delete [] mesh_input_->segmentlist;
    delete [] mesh_input_->segmentmarkerlist;
    delete [] mesh_input_->holelist;
    delete [] mesh_input_->regionlist;
#else
    std::cerr << WHERE_AM_I << " Triangle not installed" << std::endl;
#endif
}

void TriangleWrapper::transformTriangleToMesh_( const triangulateio & trimesh, Mesh & mesh ){
#ifdef HAVE_LIBTRIANGLE
    mesh.clear();

    //!  Nodes;
    int marker = 0;
    for ( int i = 0; i < trimesh.numberofpoints; i ++ ){
        if ( trimesh.pointmarkerlist != (int*)NULL ) marker = trimesh.pointmarkerlist[ i ];
        mesh.createNode( trimesh.pointlist[ i * 2 ], trimesh.pointlist[ i * 2 + 1 ], 0.0, marker );
    }

    //!  Edges / Segments
    if ( trimesh.numberofedges == 0 ){
        for ( int i = 0; i < trimesh.numberofsegments; i ++ ){
            if ( trimesh.segmentmarkerlist != (int*)NULL ) marker = trimesh.segmentmarkerlist[ i ];
            mesh.createEdge( mesh.node( trimesh.segmentlist[ i * 2 ] ), mesh.node(trimesh.segmentlist[ i * 2 + 1 ] ), marker );
        }
    } else {
        if ( trimesh.edgelist != NULL ){
            for ( int i = 0; i < trimesh.numberofedges; i ++ ){
                if ( trimesh.edgemarkerlist != (int*)NULL ) marker = trimesh.edgemarkerlist[ i ];
                mesh.createEdge( mesh.node( trimesh.edgelist[ i * 2 ] ), mesh.node( trimesh.edgelist[ i * 2 + 1 ] ), marker );
            }
        } else {
            std::cout << "Warning! " << WHERE_AM_I << " edges are not exported. Append -e flag to the triangle command." << std::endl;
        }
    }

    //! Triangles;
    int a = 0, b = 0, c = 0;
    double attribute = 0.0;
    for ( int i = 0; i < trimesh.numberoftriangles; i ++ ){
        if ( trimesh.triangleattributelist != (REAL*)NULL ) attribute = trimesh.triangleattributelist[ i ];

        a = trimesh.trianglelist[ i * 3 ];
        b = trimesh.trianglelist[ i * 3 + 1 ];
        c = trimesh.trianglelist[ i * 3 + 2 ];

        mesh.createTriangle( mesh.node( a ), mesh.node( b ), mesh.node( c ), (size_t)attribute );
    }

// //** RegionMarker
//   for (int i = 0; i < trimesh.numberofregions * 4; i += 4){
//     mesh.createRegionMarker( trimesh.regionlist[ i ], trimesh.regionlist[ i + 1 ],
// 			      trimesh.regionlist[ i + 2 ], trimesh.regionlist[ i + 3 ] );
//   }
//   for (int i = 0; i < trimesh.numberofholes * 2; i += 2){
//     mesh.createRegionMarker( trimesh.holelist[ i ], trimesh.holelist[ i + 1], -1, -1 );
//   }
#else
    std::cerr << WHERE_AM_I << " Triangle not installed" << std::endl;
#endif
}

void TriangleWrapper::transformMeshToTriangle_( const Mesh & mesh, triangulateio & trimesh ){
#ifdef HAVE_LIBTRIANGLE
    //! node section
    int nVerts = mesh.nodeCount();

    trimesh.numberofpoints = nVerts;
    trimesh.numberofpointattributes = 0;  // only one Parameter

    trimesh.pointlist = new double[ 2 * nVerts ];
    trimesh.pointmarkerlist = new int[ nVerts ];

    for ( int i = 0; i < nVerts; i ++ ){
        trimesh.pointlist[ i * 2 ]      = mesh.node( i ).x();
        trimesh.pointlist[ i * 2 + 1]   = mesh.node( i ).y();
        trimesh.pointmarkerlist[ i ]    = mesh.node( i ).marker();
    }

    //! edge section;
    int nEdges      = mesh.boundaryCount();
    int edgeCounter = 0;

    trimesh.numberofsegments    = nEdges;
    trimesh.segmentlist         = new int[ 2 * nEdges ];
    trimesh.segmentmarkerlist   = new int[ nEdges ];

    for ( int i = 0; i < nEdges; i ++ ){
        Boundary * edge = &mesh.boundary( i );
        trimesh.segmentlist[ edgeCounter * 2 ]      = edge->node( 0 ).id();
        trimesh.segmentlist[ edgeCounter * 2 + 1 ]  = edge->node( 1 ).id();
        trimesh.segmentmarkerlist[ edgeCounter ]    = edge->marker();
        edgeCounter++;
    }

    //! Holes;
    uint nHoles = holeMarker_.size();//domain.holeCount();
    trimesh.numberofholes = nHoles;
    trimesh.holelist = new double[ 2 * nHoles + 1 ];

    for ( uint i = 0; i < nHoles; i ++ ){
        trimesh.holelist[ i * 2 ] = holeMarker_[ i ].x();
        trimesh.holelist[ i * 2 + 1 ] = holeMarker_[ i ].y();
     }

    //! Regions;
    int nRegions = regionMarker_.size();//domain.regionCount();
    trimesh.numberofregions = nRegions;
    trimesh.regionlist = new double[ 4 * nRegions + 1 ];

    uint count = 0;
    for ( std::map< int, std::pair< RVector3, double > >::const_iterator it  = regionMarker_.begin();
                                                                         it != regionMarker_.end(); it ++ ){

//         std::cout << it->second.first.x() << " "
//                   << it->second.first.y() << " " << it->first << " "<< it->second.second << " "<< std::endl;
        trimesh.regionlist[ count * 4 ] = it->second.first.x();
        trimesh.regionlist[ count * 4 + 1 ] = it->second.first.y();
        trimesh.regionlist[ count * 4 + 2 ] = it->first;
        trimesh.regionlist[ count * 4 + 3 ] = it->second.second;
        count ++;
    }
//     for ( int i = 0; i < nRegions; i ++ ){
//         trimesh.regionlist[ i * 4 ] = domain.region( i ).x();
//         trimesh.regionlist[ i * 4 + 1 ] = domain.region( i ).y();
//         trimesh.regionlist[ i * 4 + 2 ] = domain.region( i ).attribute();
//         trimesh.regionlist[ i * 4 + 3 ] = domain.region( i ).dx();
//     }


#else
    std::cerr << WHERE_AM_I << " Triangle not installed" << std::endl;
#endif
}

void TriangleWrapper::addRegionMarkerTmp( int marker, const RVector3 & pos, double area ){
    
    if ( area == -1.0 ) {
        holeMarker_.push_back( pos );
    }
    else {
        regionMarker_[ marker ] = std::pair< RVector3, double > ( pos, area );
    }
}

} // namespace GIMLI