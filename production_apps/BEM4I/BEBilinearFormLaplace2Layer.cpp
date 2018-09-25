/*!
 * @file    BEBilinearFormLaplace2Layer.cpp
 * @author  Michal Merta
 * @date    August 8, 2013
 *
 */

#ifdef BEBILINEARFORMLAPLACE2LAYER_H

namespace bem4i {

template<class LO, class SC>
BEBilinearFormLaplace2Layer<LO, SC>::BEBilinearFormLaplace2Layer( ) {
}

template<class LO, class SC>
BEBilinearFormLaplace2Layer<LO, SC>::BEBilinearFormLaplace2Layer(
    const BEBilinearFormLaplace2Layer& orig
    ) {
  this->space = orig.space;
  this->quadratureOrder = orig.quadratureOrder;
  this->quadratureOrderDisjointElems = orig.quadratureOrderDisjointElems;
}

template<class LO, class SC>
BEBilinearFormLaplace2Layer<LO, SC>::BEBilinearFormLaplace2Layer(
    BESpace<LO, SC>* space,
    int* quadratureOrder,
    quadratureType quadrature,
    int* quadratureOrderDisjointElems
    ) {

  this->quadrature = quadrature;
  this->space = space;
  if ( quadratureOrder ) {
    this->quadratureOrder = quadratureOrder;
  } else {
    switch ( quadrature ) {
      case SauterSchwab:
        this->quadratureOrder = defaultQuadraturesSauterSchwab;
        break;
      case Steinbach:
        this->quadratureOrder = defaultQuadraturesSteinbach;
    }
  }

  this->quadratureOrderDisjointElems = quadratureOrderDisjointElems;

}

template<class LO, class SC>
BEBilinearFormLaplace2Layer<LO, SC>::~BEBilinearFormLaplace2Layer( ) {
}

template<class LO, class SC>
void BEBilinearFormLaplace2Layer<LO, SC>::assemble(
    const BECluster< LO, SC > * leftCluster,
    const BECluster< LO, SC > * rightCluster,
    FullMatrix< LO, SC > & matrix,
    void * voidIntegrator
    ) const {

  if ( this->space->getAnsatzFunctionType( ) == p1dis &&
      this->space->getTestFunctionType( ) == p0 ) {

    this->assembleP0P1Dis( leftCluster, rightCluster, matrix, voidIntegrator );
    return;
  } else if ( this->space->getAnsatzFunctionType( ) == p1dis &&
      this->space->getTestFunctionType( ) == p1dis ) {

    this->assembleP1DisP1Dis( leftCluster, rightCluster, matrix,
        voidIntegrator );
    return;
  } else if ( this->space->getAnsatzFunctionType( ) == p0 &&
      this->space->getTestFunctionType( ) == p0 ) {

    this->assembleP0P0( leftCluster, rightCluster, matrix, voidIntegrator );
    return;
  }

  BEIntegratorLaplace<LO, SC> * integrator =
      static_cast<BEIntegratorLaplace< LO, SC > *> ( voidIntegrator );

  const vector<LO>* innerElems = rightCluster->elems;
  const vector<LO>* outerElems = leftCluster->elems;

  // allocate local element matrices
  LO nLocalRows = this->space->getDOFsPerOuterElem( );
  LO nLocalCols = this->space->getDOFsPerInnerElem( );

  matrix.resize( this->space->getClusterOuterDOFs( leftCluster ),
      this->space->getClusterInnerDOFs( rightCluster ) );

  LO iMax = outerElems->size( );
  LO jMax = innerElems->size( );

  // vector of indices where to put values in the global matrix
  vector<LO> rowIndices( nLocalRows );
  vector<LO> colIndices( nLocalCols );
  FullMatrix<LO, SC> elemMatrix( nLocalRows, nLocalCols );

  for ( LO i = 0; i < iMax; i++ ) {
    for ( LO j = 0; j < jMax; j++ ) {
      integrator->getElemMatrix2Layer( ( *outerElems )[i], ( *innerElems )[j],
          elemMatrix );
      this->space->getClusterOuterElemDOFs( leftCluster, i, &rowIndices[0] );
      this->space->getClusterInnerElemDOFs( rightCluster, j, &colIndices[0] );
      matrix.addToPositions( rowIndices, colIndices, elemMatrix );
    }
  }
}

template<class LO, class SC>
void BEBilinearFormLaplace2Layer<LO, SC>::assembleP0P0(
    const BECluster< LO, SC > * leftCluster,
    const BECluster< LO, SC > * rightCluster,
    FullMatrix< LO, SC > & matrix,
    void * voidIntegrator
    ) const {

  BEIntegratorLaplace<LO, SC> * integrator =
      static_cast<BEIntegratorLaplace< LO, SC > *> ( voidIntegrator );

  const vector<LO> * innerElems = rightCluster->elems;
  const vector<LO> * outerElems = leftCluster->elems;
  LO iMax = outerElems->size( );
  LO jMax = innerElems->size( );

  matrix.resize( iMax, jMax );

  FullMatrix<LO, SC> elemMatrix( 1, 1 );

  for ( LO j = 0; j < jMax; ++j ) {
    for ( LO i = 0; i < iMax; ++i ) {
      integrator->getElemMatrix2Layer( outerElems->at( i ), innerElems->at( j ),
          elemMatrix );
      matrix.set( i, j, elemMatrix.get( 0, 0 ) );
    }
  }
}

template<class LO, class SC>
void BEBilinearFormLaplace2Layer<LO, SC>::assembleP0P1Dis(
    const BECluster< LO, SC > * leftCluster,
    const BECluster< LO, SC > * rightCluster,
    FullMatrix< LO, SC > & matrix,
    void * voidIntegrator
    ) const {

  BEIntegratorLaplace<LO, SC> * integrator =
      static_cast<BEIntegratorLaplace< LO, SC > *> ( voidIntegrator );

  const vector<LO> * innerElems = rightCluster->elems;
  const vector<LO> * outerElems = leftCluster->elems;
  LO iMax = outerElems->size( );
  LO jMax = innerElems->size( );

  matrix.resize( iMax, 3 * jMax );

  FullMatrix<LO, SC> elemMatrix( 1, 3 );

  for ( LO j = 0; j < jMax; ++j ) {
    for ( LO i = 0; i < iMax; ++i ) {
      integrator->getElemMatrix2Layer( outerElems->at( i ), innerElems->at( j ),
          elemMatrix );
      matrix.set( i, 3 * j, elemMatrix.get( 0, 0 ) );
      matrix.set( i, 3 * j + 1, elemMatrix.get( 0, 1 ) );
      matrix.set( i, 3 * j + 2, elemMatrix.get( 0, 2 ) );
    }
  }
}

template<class LO, class SC>
void BEBilinearFormLaplace2Layer<LO, SC>::assembleP1DisP1Dis(
    const BECluster< LO, SC > * leftCluster,
    const BECluster< LO, SC > * rightCluster,
    FullMatrix< LO, SC > & matrix,
    void * voidIntegrator
    ) const {

  BEIntegratorLaplace<LO, SC> * integrator =
      static_cast<BEIntegratorLaplace< LO, SC > *> ( voidIntegrator );

  const vector<LO> * innerElems = rightCluster->elems;
  const vector<LO> * outerElems = leftCluster->elems;
  LO iMax = outerElems->size( );
  LO jMax = innerElems->size( );

  matrix.resize( 3 * iMax, 3 * jMax );
  SC * matrixData = matrix.getData( );

  FullMatrix<LO, SC> elemMatrix( 3, 3 );
  SC * elemMatrixData = elemMatrix.getData( );

  for ( LO i = 0; i < iMax; ++i ) {
    for ( LO j = 0; j < jMax; ++j ) {

      integrator->getElemMatrix2Layer( outerElems->at( i ), innerElems->at( j ),
          elemMatrix );
      for ( int oRot = 0; oRot < 3; ++oRot ) {
        for ( int iRot = 0; iRot < 3; ++iRot ) {
          matrixData[ ( 3 * j + iRot ) * 3 * iMax + 3 * i + oRot ] =
              elemMatrixData[ iRot * 3 + oRot ];
        }
      }
    }
  }
}

///*

template<class LO, class SC>
void BEBilinearFormLaplace2Layer<LO, SC>::assembleRow(
    const BEBlockCluster< LO, SC > & block,
    LO idx,
    const std::vector<LO> & cols,
    Vector<LO, SC> & row,
    void * voidIntegrator
    ) const {

#pragma omp atomic update
  ++this->counterRow;

  if ( this->space->getTestFunctionType( ) == p0 &&
      this->space->getAnsatzFunctionType( ) == p1dis ) {
    this->assembleRowP0P1Dis( block, idx, cols, row, voidIntegrator );
    return;
  } else if ( this->space->getTestFunctionType( ) == p0 &&
      this->space->getAnsatzFunctionType( ) == p0 ) {
    this->assembleRowP0P0( block, idx, cols, row, voidIntegrator );
    return;
  } else if ( this->space->getTestFunctionType( ) == p1dis &&
      this->space->getAnsatzFunctionType( ) == p1dis ) {
    this->assembleRowP1DisP1Dis( block, idx, cols, row, voidIntegrator );
    return;
  }

  BEIntegratorLaplace<LO, SC> * integrator =
      static_cast<BEIntegratorLaplace< LO, SC > *> ( voidIntegrator );

  LO nLocalRows = this->space->getDOFsPerOuterElem( );
  LO nLocalCols = this->space->getDOFsPerInnerElem( );

  FullMatrix<LO, SC> elemMatrix( nLocalRows, nLocalCols );
  // vector of indices where to put values in the global matrix
  vector<LO> rowIndices( nLocalRows );
  vector<LO> colIndices( nLocalCols );
  LO nCols = cols.size( );
  //row.resize( nCols, true );
  row.setAll( 0.0 );
  vector< LO > testSupport;
  vector< LO > ansatzSupport;
  this->space->getOuterSupport( idx, testSupport,
      *( block.leftCluster->elems ) );

  for ( LO i = 0; i < testSupport.size( ); i++ ) {
    this->space->getOuterElemDOFs( testSupport[i], &rowIndices[0] );
    LO localRowIdx = 0;
    for ( LO auxI = 0; auxI < nLocalRows; auxI++ ) {
      if ( rowIndices[auxI] == idx ) {
        localRowIdx = auxI;
      }
    }
    for ( LO j = 0; j < nCols; j++ ) {
      this->space->getInnerSupport( cols[ j ], ansatzSupport,
          *( block.rightCluster->elems ) );
      for ( LO k = 0; k < ansatzSupport.size( ); k++ ) {
        integrator->getElemMatrix2Layer( testSupport[i], ansatzSupport[k],
            elemMatrix );

        this->space->getInnerElemDOFs( ansatzSupport[k], &colIndices[0] );
        LO localColIdx = 0;
        for ( LO auxJ = 0; auxJ < nLocalCols; auxJ++ ) {
          if ( colIndices[auxJ] == cols[ j ] ) {
            localColIdx = auxJ;
          }
        }
        row.add( j, elemMatrix.get( localRowIdx, localColIdx ) );
      }
    }
  }
}
//*/

// test to avoid multiple integration over the same pairs of elements

/*
template<class LO, class SC>
void BEBilinearFormLaplace2Layer<LO, SC>::assembleRow(
    const BEBlockCluster< LO, SC > & block,
    LO idx,
    const std::vector<LO> & cols,
    Vector<LO, SC> & row,
    void * voidIntegrator
    ) const {

  BEIntegratorLaplace<LO, SC> * integrator =
      static_cast<BEIntegratorLaplace< LO, SC > *> ( voidIntegrator );

  LO nLocalRows = this->space->getDOFsPerOuterElem( );
  LO nLocalCols = this->space->getDOFsPerInnerElem( );

  FullMatrix<LO, SC> elemMatrix( nLocalRows, nLocalCols );
  // vector of indices where to put values in the global matrix
  vector<LO> rowIndices( nLocalRows );
  vector<LO> colIndices( nLocalCols );
  LO nCols = cols.size( );

  row.setAll( 0.0 );

  vector< LO > testSupport;
  vector< LO > ansatzSupport;
  this->space->getOuterSupport( idx, testSupport,
 *( block.leftCluster->elems ) );

  LO testSuppSize = testSupport.size( );
  std::vector< std::vector< std::vector< LO > > > writeIdx( testSuppSize );
  std::vector< std::vector< std::vector< LO > > > readRowIdx( testSuppSize );
  std::vector< std::vector< std::vector< LO > > > readColIdx( testSuppSize );
  std::vector< std::vector< LO > > elemTuples( testSuppSize );

  for ( LO i = 0; i < testSuppSize; ++i ) {
    this->space->getOuterElemDOFs( testSupport[ i ], &rowIndices[ 0 ] );
    LO localRowIdx = 0;
    for ( LO auxI = 0; auxI < nLocalRows; ++auxI ) {
      if ( rowIndices[ auxI ] == idx ) {
        localRowIdx = auxI;
        break;
      }
    }

    for ( LO j = 0; j < nCols; j++ ) {
      this->space->getInnerSupport( cols[ j ], ansatzSupport,
 *( block.rightCluster->elems ) );

      for ( LO k = 0; k < ansatzSupport.size( ); ++k ) {
        this->space->getInnerElemDOFs( ansatzSupport[ k ], &colIndices[ 0 ] );
        LO localColIdx = 0;
        for ( LO auxJ = 0; auxJ < nLocalCols; ++auxJ ) {
          if ( colIndices[ auxJ ] == cols[ j ] ) {
            localColIdx = auxJ;
            break;
          }
        }

        // check elems testSupport[ i ], ansatzSupport[ k ]
        bool found = false;
        for ( LO l = 0; l < elemTuples[ i ].size( ); ++l ) {
          if ( elemTuples[ i ][ l ] == ansatzSupport[ k ] ) {

            writeIdx[ i ][ l ].push_back( j );
            readRowIdx[ i ][ l ].push_back( localRowIdx );
            readColIdx[ i ][ l ].push_back( localColIdx );
            found = true;
            break;

          }
        }        
        if ( !found ) {
          elemTuples[ i ].push_back( ansatzSupport[ k ] );
          writeIdx[ i ].push_back( std::vector< LO >{ j } );
          readRowIdx[ i ].push_back( std::vector< LO >{ localRowIdx } );
          readColIdx[ i ].push_back( std::vector< LO >{ localColIdx } );
        }
      }
    }
  }
 
  for ( LO i = 0; i < testSuppSize; ++i ) {
    for ( LO j = 0; j < elemTuples[ i ].size( ); ++j ) {

      integrator->getElemMatrix2Layer( testSupport[ i ], elemTuples[ i ][ j ],
          elemMatrix );

      for ( LO k = 0; k < writeIdx[ i ][ j ].size( ); ++k ) {
        row.add( writeIdx[ i ][ j ][ k ], elemMatrix.get(
            readRowIdx[ i ][ j ][ k ], readColIdx[ i ][ j ][ k ] ) );
      }
    }
  }

}
 */

template<class LO, class SC>
void BEBilinearFormLaplace2Layer<LO, SC>::assembleRowP0P0(
    const BEBlockCluster< LO, SC > & block,
    LO idx,
    const std::vector<LO> & cols,
    Vector<LO, SC> & row,
    void * voidIntegrator
    ) const {

  BEIntegratorLaplace<LO, SC> * integrator =
      static_cast<BEIntegratorLaplace< LO, SC > *> ( voidIntegrator );

  FullMatrix<LO, SC> elemMatrix( 1, 1 );

  for ( LO i = 0; i < block.rightCluster->nelems; ++i ) {
    integrator->getElemMatrix2Layer( idx, block.rightCluster->elems->at( i ),
        elemMatrix );
    row.set( i, elemMatrix.get( 0, 0 ) );
  }
}

template<class LO, class SC>
void BEBilinearFormLaplace2Layer<LO, SC>::assembleRowP0P1Dis(
    const BEBlockCluster< LO, SC > & block,
    LO idx,
    const std::vector<LO> & cols,
    Vector<LO, SC> & row,
    void * voidIntegrator
    ) const {

  BEIntegratorLaplace<LO, SC> * integrator =
      static_cast<BEIntegratorLaplace< LO, SC > *> ( voidIntegrator );

  FullMatrix<LO, SC> elemMatrix( 1, 3 );

  for ( LO i = 0; i < block.rightCluster->nelems; ++i ) {
    integrator->getElemMatrix2Layer( idx, block.rightCluster->elems->at( i ),
        elemMatrix );
    row.set( 3 * i, elemMatrix.get( 0, 0 ) );
    row.set( 3 * i + 1, elemMatrix.get( 0, 1 ) );
    row.set( 3 * i + 2, elemMatrix.get( 0, 2 ) );
  }
}

template<class LO, class SC>
void BEBilinearFormLaplace2Layer<LO, SC>::assembleRowP1DisP1Dis(
    const BEBlockCluster< LO, SC > & block,
    LO idx,
    const std::vector<LO> & cols,
    Vector<LO, SC> & row,
    void * voidIntegrator
    ) const {

  BEIntegratorLaplace<LO, SC> * integrator =
      static_cast<BEIntegratorLaplace< LO, SC > *> ( voidIntegrator );

  FullMatrix<LO, SC> elemMatrix( 3, 3 );

  for ( LO i = 0; i < block.rightCluster->nelems; ++i ) {
    integrator->getElemMatrix2Layer( idx / 3,
        block.rightCluster->elems->at( i ), elemMatrix );
    row.set( 3 * i, elemMatrix.get( idx % 3, 0 ) );
    row.set( 3 * i + 1, elemMatrix.get( idx % 3, 1 ) );
    row.set( 3 * i + 2, elemMatrix.get( idx % 3, 2 ) );
  }
}

template<class LO, class SC>
void BEBilinearFormLaplace2Layer<LO, SC>::assembleColumn(
    const BEBlockCluster< LO, SC > & block,
    LO idx,
    const std::vector< LO > & rows,
    Vector<LO, SC>& col,
    void * voidIntegrator
    ) const {

#pragma omp atomic update
  ++this->counterCol;

  if ( this->space->getTestFunctionType( ) == p0 &&
      this->space->getAnsatzFunctionType( ) == p1dis ) {
    this->assembleColumnP0P1Dis( block, idx, rows, col, voidIntegrator );
    return;
  } else if ( this->space->getTestFunctionType( ) == p0 &&
      this->space->getAnsatzFunctionType( ) == p0 ) {
    this->assembleColumnP0P0( block, idx, rows, col, voidIntegrator );
    return;
  } else if ( this->space->getTestFunctionType( ) == p1dis &&
      this->space->getAnsatzFunctionType( ) == p1dis ) {
    this->assembleColumnP1DisP1Dis( block, idx, rows, col, voidIntegrator );
    return;
  }

  BEIntegratorLaplace<LO, SC> * integrator =
      static_cast<BEIntegratorLaplace< LO, SC > *> ( voidIntegrator );

  LO nLocalRows = this->space->getDOFsPerOuterElem( );
  LO nLocalCols = this->space->getDOFsPerInnerElem( );

  FullMatrix<LO, SC> elemMatrix( nLocalRows, nLocalCols );
  // vector of indices where to put values in the global matrix
  vector<LO> rowIndices( nLocalRows );
  vector<LO> colIndices( nLocalCols );

  LO nRows = rows.size( );
  //col.resize( nRows, true );
  col.setAll( 0.0 );
  vector< LO > testSupport;
  vector< LO > ansatzSupport;
  this->space->getInnerSupport( idx, ansatzSupport,
      *( block.rightCluster->elems ) );

  for ( LO i = 0; i < ansatzSupport.size( ); i++ ) {
    this->space->getInnerElemDOFs( ansatzSupport[i], &colIndices[0] );
    LO localColIdx = 0;
    for ( LO auxI = 0; auxI < nLocalCols; auxI++ ) {
      if ( colIndices[auxI] == idx ) {
        localColIdx = auxI;
      }
    }
    for ( LO j = 0; j < nRows; j++ ) {
      this->space->getOuterSupport( rows[ j ], testSupport,
          *( block.leftCluster->elems ) );

      for ( LO k = 0; k < testSupport.size( ); k++ ) {
        integrator->getElemMatrix2Layer( testSupport[k], ansatzSupport[i],
            elemMatrix );

        this->space->getOuterElemDOFs( testSupport[k], &rowIndices[0] );
        LO localRowIdx = 0;
        for ( LO auxJ = 0; auxJ < nLocalRows; auxJ++ ) {
          if ( rowIndices[auxJ] == rows[ j ] ) {
            localRowIdx = auxJ;
          }
        }
        col.add( j, elemMatrix.get( localRowIdx, localColIdx ) );
      }
    }
  }
}

template<class LO, class SC>
void BEBilinearFormLaplace2Layer<LO, SC>::assembleColumnP0P0(
    const BEBlockCluster< LO, SC > & block,
    LO idx,
    const std::vector<LO> & rows,
    Vector<LO, SC> & col,
    void * voidIntegrator
    ) const {

  BEIntegratorLaplace<LO, SC> * integrator =
      static_cast<BEIntegratorLaplace< LO, SC > *> ( voidIntegrator );

  FullMatrix<LO, SC> elemMatrix( 1, 1 );

  for ( LO i = 0; i < block.leftCluster->nelems; ++i ) {
    integrator->getElemMatrix2Layer( block.leftCluster->elems->at( i ), idx,
        elemMatrix );
    col.set( i, elemMatrix.get( 0, 0 ) );
  }
}

template<class LO, class SC>
void BEBilinearFormLaplace2Layer<LO, SC>::assembleColumnP0P1Dis(
    const BEBlockCluster< LO, SC > & block,
    LO idx,
    const std::vector<LO> & rows,
    Vector<LO, SC> & col,
    void * voidIntegrator
    ) const {

  BEIntegratorLaplace<LO, SC> * integrator =
      static_cast<BEIntegratorLaplace< LO, SC > *> ( voidIntegrator );

  FullMatrix<LO, SC> elemMatrix( 1, 3 );

  for ( LO i = 0; i < block.leftCluster->nelems; ++i ) {
    integrator->getElemMatrix2Layer( block.leftCluster->elems->at( i ), idx / 3,
        elemMatrix );
    col.set( i, elemMatrix.get( 0, idx % 3 ) );
  }
}

template<class LO, class SC>
void BEBilinearFormLaplace2Layer<LO, SC>::assembleColumnP1DisP1Dis(
    const BEBlockCluster< LO, SC > & block,
    LO idx,
    const std::vector<LO> & rows,
    Vector<LO, SC> & col,
    void * voidIntegrator
    ) const {

  BEIntegratorLaplace<LO, SC> * integrator =
      static_cast<BEIntegratorLaplace< LO, SC > *> ( voidIntegrator );

  FullMatrix<LO, SC> elemMatrix( 3, 3 );

  for ( LO i = 0; i < block.leftCluster->nelems; ++i ) {
    integrator->getElemMatrix2Layer(
        block.leftCluster->elems->at( i ), idx / 3, elemMatrix );
    col.set( 3 * i, elemMatrix.get( 0, idx % 3 ) );
    col.set( 3 * i + 1, elemMatrix.get( 1, idx % 3 ) );
    col.set( 3 * i + 2, elemMatrix.get( 2, idx % 3 ) );
  }
}

template<class LO, class SC>
void BEBilinearFormLaplace2Layer<LO, SC>::assemble(
    FullMatrix<LO, SC>& matrix
    ) const {
#if N_MIC > 0
  this->assembleMIC( matrix );
#else

  if ( this->space->getAnsatzFunctionType( ) == p1 &&
      this->space->getTestFunctionType( ) == p0 ) {

    this->assembleP0P1( matrix );
    return;
  } else if ( this->space->getAnsatzFunctionType( ) == p1 &&
      this->space->getTestFunctionType( ) == p1 ) {

    this->assembleP1P1( matrix );
    return;
  } else if ( this->space->getAnsatzFunctionType( ) == p0 &&
      this->space->getTestFunctionType( ) == p0 ) {

    this->assembleP0P0( matrix );
    return;
  }

  vector<LO> innerElems = this->space->getInnerElems( );
  vector<LO> outerElems = this->space->getOuterElems( );

  // allocate local element matrices
  LO nLocalRows = this->space->getDOFsPerOuterElem( );
  LO nLocalCols = this->space->getDOFsPerInnerElem( );

  matrix.resize( this->space->getOuterDOFs( ),
      this->space->getInnerDOFs( ) );

  LO iMax = outerElems.size( );
  LO jMax = innerElems.size( );
#ifdef VERBOSE
  ProgressMonitor::init( "Assembling Laplace K", iMax * jMax );
#endif

#pragma omp parallel shared(matrix)
  {
    BEIntegratorLaplace<LO, SC> integrator( this->space, this->quadratureOrder,
        this->quadrature, this->quadratureOrderDisjointElems );
    // vector of indices where to put values in the global matrix
    FullMatrix<LO, SC>* matrixBuffer[BUFFER_SIZE];
    vector<LO>* rowIndicesBuffer[BUFFER_SIZE];
    vector<LO>* colIndicesBuffer[BUFFER_SIZE];
    for ( int i = 0; i < BUFFER_SIZE; i++ ) {
      matrixBuffer[i] = new FullMatrix<LO, SC>( nLocalRows, nLocalCols );
      rowIndicesBuffer[i] = new vector<LO>( nLocalRows );
      colIndicesBuffer[i] = new vector<LO>( nLocalCols );
    }
    int counter = 0;

#pragma omp for collapse(2)
    for ( LO i = 0; i < iMax; i++ ) {
      //#pragma omp for
      for ( LO j = 0; j < jMax; j++ ) {
        integrator.getElemMatrix2Layer( outerElems[i], innerElems[j],
            *( matrixBuffer[counter] ) );
        this->space->getOuterElemDOFs( outerElems[i],
            &( *( rowIndicesBuffer[counter] ) )[0] );
        this->space->getInnerElemDOFs( innerElems[j],
            &( *( colIndicesBuffer[counter] ) )[0] );
        counter++;

        if ( counter == BUFFER_SIZE ) {
          counter = 0;

          for ( int k = 0; k < BUFFER_SIZE; k++ ) {
            matrix.addToPositionsAtomic( *( rowIndicesBuffer[k] ),
                *( colIndicesBuffer[k] ), *( matrixBuffer[k] ) );
          }

#ifdef VERBOSE
          ProgressMonitor::step( BUFFER_SIZE );
#endif
        }
      }
    }

    for ( int i = 0; i < counter; i++ ) {
      matrix.addToPositionsAtomic( *( rowIndicesBuffer[i] ),
          *( colIndicesBuffer[i] ), *( matrixBuffer[i] ) );
    }

#ifdef VERBOSE
    ProgressMonitor::step( counter );
#endif
    for ( int i = 0; i < BUFFER_SIZE; i++ ) {
      delete matrixBuffer[i];
      delete rowIndicesBuffer[i];
      delete colIndicesBuffer[i];
    }

  }
#endif
}

template<class LO, class SC>
void BEBilinearFormLaplace2Layer<LO, SC>::assembleP0P1(
    FullMatrix<LO, SC> & matrix
    ) const {


  LO nNodes = this->space->getMesh( )->getNNodes( );
  LO nElems = this->space->getMesh( )->getNElements( );

  matrix.resize( nElems, nNodes, false );
  SC * matrixData = matrix.getData( );

#pragma omp parallel for schedule( static, 32 )
  for ( LO j = 0; j < nNodes; ++j ) {
    for ( LO i = 0; i < nElems; ++i ) {
      matrixData[ j * nElems + i ] = 0.0;
    }
  }

#pragma omp parallel
  {
    BEIntegratorLaplace<LO, SC> integrator( this->space, this->quadratureOrder,
        this->quadrature, this->quadratureOrderDisjointElems );

    FullMatrix< LO, SC > elemMatrix( 1, 3 );
    LO elem[ 3 ];

#pragma omp for schedule( dynamic, 8 )
    for ( LO i = 0; i < nElems; ++i ) {
      for ( LO j = 0; j < nElems; ++j ) {
        //int dummy, type;
        //integrator.getSauterSchwabQuadratureType( j, i, type, dummy, dummy );
        //if( type != 1 ) continue;
        //if( i != 0 || j != 15 ) continue;

        integrator.getElemMatrix2Layer( i, j, elemMatrix );
        this->space->getMesh( )->getElement( j, elem );

        //if( i == 0 && j == 15 ) elemMatrix.print();

        //#pragma omp atomic update
        matrixData[ elem[ 0 ] * nElems + i ] += elemMatrix.get( 0, 0 );
        //#pragma omp atomic update
        matrixData[ elem[ 1 ] * nElems + i ] += elemMatrix.get( 0, 1 );
        //#pragma omp atomic update
        matrixData[ elem[ 2 ] * nElems + i ] += elemMatrix.get( 0, 2 );

        //std::cout << i << " " << j << std::endl;
        //elemMatrix.print();
      }
    }
  } // end omp parallel
}

template<class LO, class SC>
void BEBilinearFormLaplace2Layer<LO, SC>::assembleP1P1(
    FullMatrix<LO, SC> & matrix
    ) const {

  LO nNodes = this->space->getMesh( )->getNNodes( );
  LO nElems = this->space->getMesh( )->getNElements( );

  matrix.resize( nNodes, nNodes, false );
  SC * matrixData = matrix.getData( );

#pragma omp parallel for schedule( static, 32 )
  for ( LO j = 0; j < nNodes; ++j ) {
    for ( LO i = 0; i < nNodes; ++i ) {
      matrixData[ j * nNodes + i ] = 0.0;
    }
  }

#pragma omp parallel
  {
    BEIntegratorLaplace<LO, SC> integrator( this->space,
        this->quadratureOrder, this->quadrature,
        this->quadratureOrderDisjointElems );

    FullMatrix< LO, SC > elemMatrix( 3, 3 );
    LO iElem[ 3 ], oElem[ 3 ];

    SC * elemMatrixData = elemMatrix.getData( );

#pragma omp for collapse( 2 ) schedule( dynamic, 32 )
    for ( LO i = 0; i < nElems; ++i ) { // i outer row
      for ( LO j = 0; j < nElems; ++j ) { // j inner col
        integrator.getElemMatrix2Layer( i, j, elemMatrix );
        this->space->getMesh( )->getElement( i, oElem );
        this->space->getMesh( )->getElement( j, iElem );

        for ( int oRot = 0; oRot < 3; ++oRot ) {
          for ( int iRot = 0; iRot < 3; ++iRot ) {
#pragma omp atomic update
            matrixData[ iElem[ iRot ] * nNodes + oElem[ oRot ] ] +=
                elemMatrixData[ iRot * 3 + oRot ];
          }
        }
      }
    }
  } // end omp parallel
}

template<class LO, class SC>
void BEBilinearFormLaplace2Layer<LO, SC>::assembleP0P0(
    FullMatrix<LO, SC> & matrix
    ) const {

  LO nElems = this->space->getMesh( )->getNElements( );

  matrix.resize( nElems, nElems, false );
  SC * matrixData = matrix.getData( );

#pragma omp parallel
  {
    BEIntegratorLaplace<LO, SC> integrator( this->space, this->quadratureOrder,
        this->quadrature, this->quadratureOrderDisjointElems );

    FullMatrix< LO, SC > elemMatrix( 1, 1 );

#pragma omp for schedule( dynamic, 8 )
    for ( LO j = 0; j < nElems; ++j ) {
      for ( LO i = 0; i < nElems; ++i ) {
        integrator.getElemMatrix2Layer( i, j, elemMatrix );
        matrixData[ j * nElems + i ] = elemMatrix.get( 0, 0 );
      }
    }
  } // end omp parallel
}

template<class LO, class SC>
void BEBilinearFormLaplace2Layer<LO, SC>::assemble(
    FMMMatrix<LO, SC>& matrix
    ) const {

  // assemble nonadmissible blocks

  matrix.setNRows( this->space->getOuterDOFs( ) );
  matrix.setNCols( this->space->getInnerDOFs( ) );

  FastBESpace<LO, SC>* fastSpace =
      static_cast<FastBESpace<LO, SC>*> ( this->space );

  // iterate through nonadmissible leaves and assemble appropriate part of matrix
  std::vector<BEBlockCluster<LO, SC>* >* nonAdmLeaves =
      fastSpace->getNonadmissibleLeaves( );
  matrix.setNonadmissibleLeaves( *nonAdmLeaves );
  int nNonAdm = nonAdmLeaves->size( );
  matrix.resizeNonAdmBlocks( nNonAdm );

  std::cout << "Number of nonadmissible blocks: " << nNonAdm << std::endl;
  std::cout << "Number of admissible blocks: " <<
      fastSpace->getAdmissibleLeaves( )->size( ) << std::endl;

#pragma omp parallel
  {
    BEBlockCluster<LO, SC>* block;
    BECluster<LO, SC>* leftCluster;
    BECluster<LO, SC>* rightCluster;
    void * voidIntegrator = createIntegrator( );

#pragma omp for schedule(dynamic)
    for ( int i = 0; i < nNonAdm; i++ ) {
      block = ( *nonAdmLeaves )[i];
      leftCluster = block->leftCluster;
      rightCluster = block->rightCluster;
      FullMatrix<LO, SC> *fullBlock = new FullMatrix<LO, SC>( );
      assemble( leftCluster, rightCluster, *fullBlock, voidIntegrator );

      matrix.addNonadmissibleBlock( fullBlock, i );
    }

    this->destroyIntegrator( voidIntegrator );
  }

  // assemble FMM kernel for admissible blocks
  FMMKernelLaplace2Layer<LO, SC> *kernel =
      new FMMKernelLaplace2Layer<LO, SC>( fastSpace, fastSpace->getNMax( ),
      fastSpace->getStartLevel( ), fastSpace->getQuadOrder( ) );
  matrix.setKernel( kernel );

}

// INTEL XEON PHI SPECIFIC FUNCTIONALITY

template<class LO, class SC>
void BEBilinearFormLaplace2Layer<LO, SC>::assembleP0P1MIC(
    FullMatrix<LO, SC> & matrix
    ) const {

#if N_MIC > 0

  // preallocate data for numerical integration
  int qOrderOuter = this->quadratureOrderDisjointElems[ 0 ];
  int qOrderInner = this->quadratureOrderDisjointElems[ 1 ];
  int outerPoints = quadSizes[ this->quadratureOrderDisjointElems[ 0 ] ];
  int innerPoints = quadSizes[ this->quadratureOrderDisjointElems[ 1 ] ];

  SCVT * vOutW = (SCVT *) _mm_malloc(
      outerPoints * innerPoints * sizeof ( SCVT ), DATA_ALIGN );
  SCVT * vInW = (SCVT *) _mm_malloc(
      outerPoints * innerPoints * sizeof ( SCVT ), DATA_ALIGN );
  SCVT * outerX1ref = (SCVT *) _mm_malloc(
      outerPoints * innerPoints * sizeof ( SCVT ), DATA_ALIGN );
  SCVT * outerX2ref = (SCVT *) _mm_malloc(
      outerPoints * innerPoints * sizeof ( SCVT ), DATA_ALIGN );
  SCVT * innerX1ref = (SCVT *) _mm_malloc(
      outerPoints * innerPoints * sizeof ( SCVT ), DATA_ALIGN );
  SCVT * innerX2ref = (SCVT *) _mm_malloc(
      outerPoints * innerPoints * sizeof ( SCVT ), DATA_ALIGN );
  SCVT * phi1Values = (SCVT *) _mm_malloc(
      outerPoints * innerPoints * sizeof ( SCVT ), DATA_ALIGN );
  SCVT * phi2Values = (SCVT *) _mm_malloc(
      outerPoints * innerPoints * sizeof ( SCVT ), DATA_ALIGN );
  SCVT * phi3Values = (SCVT *) _mm_malloc(
      outerPoints * innerPoints * sizeof ( SCVT ), DATA_ALIGN );

  int counter = 0;
  double * qPointsIn = quadPoints[ this->quadratureOrderDisjointElems[ 1 ] ];
  for ( int i = 0; i < outerPoints; ++i ) {
    for ( int j = 0; j < innerPoints; ++j ) {
      vOutW[ counter ] =
          (SCVT) quadWeights[ this->quadratureOrderDisjointElems[ 0 ] ][ i ];
      vInW[ counter ] =
          (SCVT) quadWeights[ this->quadratureOrderDisjointElems[ 1 ] ][ j ];
      outerX1ref[ counter ] = (SCVT) quadPoints_X1[ qOrderOuter ][ i ];
      outerX2ref[ counter ] = (SCVT) quadPoints_X2[ qOrderOuter ][ i ];
      innerX1ref[ counter ] = (SCVT) quadPoints_X1[ qOrderInner ][ j ];
      innerX2ref[ counter ] = (SCVT) quadPoints_X2[ qOrderInner ][ j ];
      phi1Values[ counter ] = 1.0 - qPointsIn[ 2 * j ] - qPointsIn[ 2 * j + 1 ];
      phi2Values[ counter ] = qPointsIn[ 2 * j ];
      phi3Values[ counter ] = qPointsIn[ 2 * j + 1 ];
      ++counter;
    }
  }

  // prepare data to be sent to mic
  LO nNodes = this->space->getMesh( )->getNNodes( );
  LO nElems = this->space->getMesh( )->getNElements( );
  SCVT * nodes = this->space->getMesh( )->getNodes( )->data( );
  LO * elems = this->space->getMesh( )->getElements( )->data( );
  SCVT * areas = this->space->getMesh( )->getAreas( )->data( );
  SCVT * normals = this->space->getMesh( )->getNormals( )->data( );

  matrix.resize( nElems, nNodes, false );
  SC * matrixData = matrix.getData( );
#pragma omp parallel for schedule( static, 32 )
  for ( LO j = 0; j < nNodes; ++j ) {
    for ( LO i = 0; i < nElems; ++i ) {
      matrixData[ j * nElems + i ] = 0.0;
    }
  }

  // divide the global matrix among available MICs
  double CPUrelative2MIC = 2.4;
  double totalPower = N_MIC + CPUrelative2MIC;
  LO nMICRows = nElems / totalPower;
  LO nRowsPerMIC[ N_MIC ];
  long dataLength[ N_MIC ];
  long dataInBytes[ N_MIC ];
  for ( LO i = 0; i < N_MIC; ++i ) {
    nRowsPerMIC[ i ] = nMICRows;
    dataLength[ i ] = nMICRows * nNodes;
    dataInBytes[ i ] = dataLength[ i ] * sizeof ( SC );
  }
  //nRowsPerMIC[ N_MIC - 1 ] += nElems % N_MIC;
  //dataLength[ N_MIC - 1] += ( nElems % N_MIC ) * nNodes;
  //dataInBytes[ N_MIC - 1] = dataLength[ N_MIC - 1 ] * sizeof ( SC );
  LO nSubmatrices =
      std::ceil( ( (SCVT) dataInBytes[ N_MIC - 1 ] ) / MIC_CHUNK );

  LO nRowsPerSubmatrix[ N_MIC ];
  LO remainingRowsPerMIC[ N_MIC ];
  for ( LO i = 0; i < N_MIC; ++i ) {
    nRowsPerSubmatrix[ i ] = nRowsPerMIC[ i ] / nSubmatrices;
    remainingRowsPerMIC[ i ] = nRowsPerMIC[ i ] % nSubmatrices;
  }

  SC * matrixBuffers[ 2 ][ N_MIC ];
  long bufferLengths[ N_MIC ];
  for ( LO i = 0; i < N_MIC; ++i ) {
    bufferLengths[ i ] = ( nRowsPerSubmatrix[ i ] + remainingRowsPerMIC[ i ] )
        * nNodes;
    matrixBuffers[ 0 ][ i ] =
        (SC *) _mm_malloc( bufferLengths[ i ] * sizeof ( SC ), DATA_ALIGN );
    matrixBuffers[ 1 ][ i ] =
        (SC *) _mm_malloc( bufferLengths[ i ] * sizeof ( SC ), DATA_ALIGN );
  }

  LO numCPUThreads = 0;

#pragma omp parallel
  {
#pragma omp single
    numCPUThreads = omp_get_num_threads( ) - N_MIC;
    if ( numCPUThreads < 0 ) numCPUThreads = 1;
  }

  std::vector< std::vector< LO > * > rowIdx;
  rowIdx.resize( numCPUThreads );
  std::vector<std::vector< LO > * > colIdx;
  colIdx.resize( numCPUThreads );
  std::vector< std::vector< SC > * > values;
  values.resize( numCPUThreads );

  for ( int i = 0; i < numCPUThreads; ++i ) {
    rowIdx[ i ] = new std::vector< LO >;
    colIdx[ i ] = new std::vector< LO >;
    values[ i ] = new std::vector< SC >;
    rowIdx[ i ]->reserve( 200 * nElems / numCPUThreads );
    colIdx[ i ]->reserve( 200 * nElems / numCPUThreads );
    values[ i ]->reserve( 200 * nElems / numCPUThreads );
  }

#pragma omp parallel num_threads( N_MIC )
  {
    int device = omp_get_thread_num( );
    SC * myData1 = matrixBuffers[ 0 ][ device ];
    SC * myData2 = matrixBuffers[ 1 ][ device ];

#pragma offload_transfer target( mic : device ) \
    in( myData1 : length( bufferLengths[ device ] ) alloc_if( 1 ) \
    free_if( 0 ) ) \
    in( myData2 : length( bufferLengths[ device ] ) alloc_if( 1 ) \
    free_if( 0 ) ) \
    in( nodes : length( 3 * nNodes ) alloc_if( 1 ) free_if( 0 ) ) \
    in( elems : length( 3 * nElems ) alloc_if( 1 ) free_if( 0 ) ) \
    in( areas : length( nElems ) alloc_if( 1 ) free_if( 0 ) ) \
    in( normals : length( 3 * nElems ) alloc_if( 1 ) free_if( 0 ) ) \
    in( phi1Values : length( outerPoints * innerPoints ) \
    alloc_if(1) free_if(0)) \
    in( phi2Values : length( outerPoints * innerPoints ) \
    alloc_if(1) free_if(0)) \
    in( phi3Values : length( outerPoints * innerPoints ) \
    alloc_if(1) free_if(0)) \
    in( vOutW : length( outerPoints * innerPoints ) \
    alloc_if( 1 ) free_if( 0 ) ) \
    in( vInW : length( outerPoints * innerPoints ) \
    alloc_if( 1 ) free_if( 0 ) ) \
    in( nRowsPerSubmatrix : alloc_if( 1 ) free_if( 0 ) ) \
    in( remainingRowsPerMIC : alloc_if( 1 ) free_if( 0 ) ) \
    in( outerX1ref : length( outerPoints * innerPoints ) \
    alloc_if( 1 ) free_if( 0 ) ) \
    in( outerX2ref : length( outerPoints * innerPoints ) \
    alloc_if( 1 ) free_if( 0 ) ) \
    in( innerX1ref : length( outerPoints * innerPoints ) \
    alloc_if( 1 ) free_if( 0 ) ) \
    in( innerX2ref : length( outerPoints * innerPoints ) \
    alloc_if( 1 ) free_if( 0 ) ) \
    in( this : alloc_if( 1 ) free_if( 0 ) )
  }

#pragma omp parallel
  {
    if ( omp_get_thread_num( ) < N_MIC ) {

      int device = omp_get_thread_num( );
      // double buffering (one buffer for computation, one buffer sent to cpu)
      for ( LO s = 0; s <= nSubmatrices; ++s ) {

        if ( s != nSubmatrices ) {
          SC * computationData = matrixBuffers[ s % 2 ][ device ];


#pragma offload target( mic : device ) \
      in( computationData : length( 0 ) alloc_if( 0 ) free_if( 0 ) ) \
      signal( matrixBuffers[ s % 2 ][ device ] ) \
      in( nodes : length( 0 ) alloc_if(0) free_if(0) ) \
      in( elems : length( 0 ) alloc_if( 0 ) free_if( 0 ) ) \
      in( areas : length( 0 ) alloc_if( 0 ) free_if( 0 ) ) \
      in( normals : length(0) alloc_if( 0 ) free_if( 0 ) ) \
      in( phi1Values : length( 0 ) alloc_if(0) free_if(0)) \
      in( phi2Values : length( 0 ) alloc_if(0) free_if(0)) \
      in( phi3Values : length( 0 ) alloc_if(0) free_if(0)) \
      in( vOutW : length( 0 ) alloc_if( 0 ) free_if( 0 ) ) \
      in( vInW : length( 0 ) alloc_if( 0 ) free_if( 0 ) ) \
      in( nRowsPerSubmatrix :  alloc_if( 0 ) free_if( 0 ) ) \
      in( remainingRowsPerMIC : alloc_if( 0 ) free_if( 0 ) ) \
      in( outerX1ref : length( 0 ) alloc_if( 0 ) free_if( 0 ) ) \
      in( outerX2ref : length( 0 ) alloc_if( 0 ) free_if( 0 ) ) \
      in( innerX1ref : length( 0 ) alloc_if( 0 ) free_if( 0 ) ) \
      in( innerX2ref : length( 0 ) alloc_if( 0 ) free_if( 0 ) ) \
      in( s ) \
      in( this : length( 0 ) alloc_if( 0 ) free_if( 0 ) )
          {

#pragma omp parallel for schedule( static, 32 ) num_threads( N_MIC_THREADS )
#pragma vector aligned(computationData)
            for ( LO i = 0; i < bufferLengths[ device ]; ++i ) {
              computationData[ i ] = 0.0;
            }

#pragma omp parallel num_threads( N_MIC_THREADS )
            {
              SCVT * outerX1 = (SCVT *) _mm_malloc(
                  outerPoints * innerPoints * sizeof ( SCVT ), DATA_ALIGN );
              SCVT * outerX2 = (SCVT *) _mm_malloc(
                  outerPoints * innerPoints * sizeof ( SCVT ), DATA_ALIGN );
              SCVT * outerX3 = (SCVT *) _mm_malloc(
                  outerPoints * innerPoints * sizeof ( SCVT ), DATA_ALIGN );
              SCVT * innerX1 = (SCVT *) _mm_malloc(
                  outerPoints * innerPoints * sizeof ( SCVT ), DATA_ALIGN );
              SCVT * innerX2 = (SCVT *) _mm_malloc(
                  outerPoints * innerPoints * sizeof ( SCVT ), DATA_ALIGN );
              SCVT * innerX3 = (SCVT *) _mm_malloc(
                  outerPoints * innerPoints * sizeof ( SCVT ), DATA_ALIGN );

              SC elemMatrix[ 3 ] = { 0.0, 0.0, 0.0 };
              LO startRow = device * nMICRows + s * nRowsPerSubmatrix[ device ];
              LO endRow = startRow + nRowsPerSubmatrix[ device ];
              endRow +=
                  ( s == nSubmatrices - 1 ) ? remainingRowsPerMIC[ device ] : 0;

#pragma omp for collapse( 2 ) schedule( dynamic, 32 )
#pragma novector
              for ( LO i = startRow; i < endRow; ++i ) {
                for ( LO j = 0; j < nElems; ++j ) {
                  if ( !BEIntegrator<LO, SC, BEIntegratorLaplace<LO, SC> >::
                      areElementsDisjointOnMIC( i, j, nodes, elems ) ) {
                    continue;
                  }

                  BEIntegratorLaplace<LO, SC >::computeElemMatrix2LayerP0P1MIC(
                      nodes, elems, areas, normals, i, j, qOrderOuter,
                      qOrderInner, outerX1ref, outerX2ref, innerX1ref,
                      innerX2ref, outerX1, outerX2, outerX3, innerX1, innerX2,
                      innerX3, phi1Values, phi2Values, phi3Values, vOutW, vInW,
                      elemMatrix );

#pragma omp atomic update
                  computationData[ elems[ 3 * j ] *
                      ( endRow - startRow ) + i - startRow ] += elemMatrix[ 0 ];
#pragma omp atomic update
                  computationData[ elems[ 3 * j + 1 ] *
                      ( endRow - startRow ) + i - startRow ] += elemMatrix[ 1 ];
#pragma omp atomic update
                  computationData[ elems[ 3 * j + 2 ] *
                      ( endRow - startRow ) + i - startRow ] += elemMatrix[ 2 ];
                }
              }
              _mm_free( outerX1 );
              _mm_free( outerX2 );
              _mm_free( outerX3 );
              _mm_free( innerX1 );
              _mm_free( innerX2 );
              _mm_free( innerX3 );
            }
          }
        }

        // transfer data to CPU
        if ( s > 0 ) {
          SC * outputBuffer = matrixBuffers[ ( s - 1 ) % 2 ][ device ];

#pragma offload_transfer target( mic : device ) \
        wait( matrixBuffers[ ( s - 1 ) % 2 ][ device ]) \
        out( outputBuffer : length( bufferLengths[ device ] ) )

          for ( LO k = 0; k < nNodes; ++k ) {
            LO rem = ( s == nSubmatrices ) ? remainingRowsPerMIC[ device ] : 0;
            memcpy( matrixData + k * nElems + device * nMICRows +
                ( s - 1 ) * nRowsPerSubmatrix[ device ],
                matrixBuffers[ ( s - 1 ) % 2 ][ device ] +
                k * ( nRowsPerSubmatrix[ device ] + rem ),
                ( nRowsPerSubmatrix[ device ] + rem ) * sizeof ( SC ) );
          }
        }
      }

      SC* myData1 = matrixBuffers[0][device];
      SC* myData2 = matrixBuffers[1][device];
#pragma offload_transfer target( mic : device ) \
      nocopy( myData1 : alloc_if( 0 ) free_if( 1 ) ) \
      nocopy( myData2 : alloc_if( 0 ) free_if( 1 ) ) \
      nocopy( nodes : alloc_if( 0 ) free_if( 1 ) ) \
      nocopy( elems : alloc_if( 0 ) free_if( 1 ) ) \
      nocopy( areas : alloc_if( 0 ) free_if( 1 ) ) \
      nocopy( normals : alloc_if( 0 ) free_if( 1 )) \
      nocopy( vOutW : alloc_if( 0 ) free_if( 1 ) ) \
      nocopy( vInW : alloc_if( 0 ) free_if( 1 ) ) \
      nocopy( outerX1ref : alloc_if( 0 ) free_if( 1 ) ) \
      nocopy( outerX2ref : alloc_if( 0 ) free_if( 1 ) ) \
      nocopy( innerX1ref : alloc_if( 0 ) free_if( 1 ) ) \
      nocopy( innerX2ref : alloc_if( 0 ) free_if( 1 ) ) \
      nocopy( phi1Values : length( 0 ) alloc_if(0) free_if(1)) \
      nocopy( phi2Values : length( 0 ) alloc_if(0) free_if(1)) \
      nocopy( phi3Values : length( 0 ) alloc_if(0) free_if(1))
      std::cout << "MIC done" << std::endl;
    } else {

#pragma omp single nowait
      {

        for ( LO i = 0; i < N_MIC * nMICRows; ++i ) {
#pragma omp task
          {
            int myThreadNum = omp_get_thread_num( );
            BEIntegratorLaplace<LO, SC> integrator( this->space,
                this->quadratureOrder, this->quadrature,
                this->quadratureOrderDisjointElems );
            FullMatrix< LO, SC > elemMatrix( 1, 3, true );

            for ( LO j = 0; j < nElems; ++j ) {
              if ( BEIntegrator<LO, SC, BEIntegratorLaplace<LO, SC> >::
                  areElementsDisjointOnMIC( i, j, nodes, elems ) ) {
                continue;
              }

              integrator.getElemMatrix2Layer( i, j, elemMatrix );

              rowIdx[ myThreadNum - N_MIC]->push_back( i );
              rowIdx[ myThreadNum - N_MIC ]->push_back( i );
              rowIdx[ myThreadNum - N_MIC ]->push_back( i );
              colIdx[ myThreadNum - N_MIC ]->push_back( elems[ 3 * j ] );
              colIdx[ myThreadNum - N_MIC ]->push_back( elems[ 3 * j + 1 ] );
              colIdx[ myThreadNum - N_MIC ]->push_back( elems[ 3 * j + 2 ] );
              values[ myThreadNum - N_MIC ]->push_back(
                  elemMatrix.get( 0, 0 ) );
              values[ myThreadNum - N_MIC ]->push_back(
                  elemMatrix.get( 0, 1 ) );
              values[ myThreadNum - N_MIC ]->push_back(
                  elemMatrix.get( 0, 2 ) );
            } // end for j elems
          } // end omp task
        } // end i elems

        LO startRow = N_MIC * nMICRows;

        for ( LO i = startRow; i < nElems; ++i ) {
#pragma omp task
          {
            int myThreadNum = omp_get_thread_num( );
            BEIntegratorLaplace<LO, SC> integrator( this->space,
                this->quadratureOrder, this->quadrature,
                this->quadratureOrderDisjointElems );
            FullMatrix< LO, SC > elemMatrix( 1, 3, true );

            for ( LO j = 0; j < nElems; ++j ) {
              /*if ( !BEIntegrator<LO, SC, BEIntegratorLaplace<LO, SC> >::
                  areElementsDisjointOnMIC( i, j, nodes, elems ) ) {
                continue;
              }*/

              integrator.getElemMatrix2Layer( i, j, elemMatrix );


#pragma omp atomic update
              matrixData[ elems[ 3 * j ] *
                  nElems + i ] += elemMatrix.get( 0, 0 );
#pragma omp atomic update
              matrixData[ elems[ 3 * j + 1 ] *
                  nElems + i ] += elemMatrix.get( 0, 1 );
#pragma omp atomic update
              matrixData[ elems[ 3 * j + 2 ] *
                  nElems + i ] += elemMatrix.get( 0, 2 );

            } // end for j elems
          } // end omp task
        } // end i elems
        std::cout << "CPU done" << std::endl;
      } // end omp single nowait

    } // end else ( thread_num >= N_MIC )
#pragma omp barrier
  } // end omp parallel

#pragma omp parallel num_threads( numCPUThreads )
  {
    int myThreadNum = omp_get_thread_num( );
    for ( LO j = 0; j < rowIdx[ myThreadNum ]->size( ); ++j ) {
#pragma omp atomic update
      matrixData[ colIdx[ myThreadNum ]->at( j ) * nElems +
          rowIdx[ myThreadNum ]->at( j ) ] += values[ myThreadNum ]->at( j );
    }
  }

  for ( LO i = 0; i < N_MIC; ++i ) {
    _mm_free( matrixBuffers[ 0 ][ i ] );
    _mm_free( matrixBuffers[ 1 ][ i ] );
  }
  _mm_free( vOutW );
  _mm_free( vInW );
  _mm_free( outerX1ref );
  _mm_free( outerX2ref );
  _mm_free( innerX1ref );
  _mm_free( innerX2ref );
  _mm_free( phi1Values );
  _mm_free( phi2Values );
  _mm_free( phi3Values );

  for ( int i = 0; i < numCPUThreads; ++i ) {
    delete rowIdx[ i ];
    delete colIdx[ i ];
    delete values[ i ];
  }

#endif
}

}

#endif
