//==================================================================================================
//  Radio.hpp is the Radio class file for Radio2csv.
//
//  Author:  Copyright (c) 2007-2017 by Dean K. Gibson.
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, version 2 of the License.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License along
//  with this program; if not, write to the Free Software Foundation, Inc.,
//  59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef  _MSC_VER
// Rather than compile with a lower warning level, Visual Studio C++ compilation is
// done with a "-Wall" warning level, and then warnings that are deemed unneeded are
// suppressed and DOCUMENTED.  This allows for more comprehensive error checking.
# pragma  warning( disable: 4996 )  // -W1 "was declared deprecated"
# pragma  warning( disable: 4100 )  // -W4 "unreferenced formal parameter"
# pragma  warning( disable: 4710 )  // -Wall "function not inlined"
# pragma  warning( disable: 4514 )  // -Wall "unreferenced inline function has been removed"
# pragma  warning( disable: 4711 )  // -Wall "function ... selected for automatic inline expansion"
# define  strdup  _strdup
  typedef unsigned __int8  uint8_t;
  typedef unsigned __int16 uint16_t;
  typedef unsigned __int32 uint32_t;
  typedef unsigned __int64 uint64_t;
#else
# include <stdint.h>
# ifndef __cdecl
#   define __cdecl
# endif
#endif

#ifdef  HOST_MSB_FIRST  // big-endian hosts
# define LOW2HI_ORDER( LOW, HIGH )  HIGH, LOW
# define HI2LOW_ORDER( HIGH, LOW )  HIGH, LOW
#else
# define LOW2HI_ORDER( LOW, HIGH )  LOW, HIGH
# define HI2LOW_ORDER( HIGH, LOW )  LOW, HIGH
#endif

#define COUNT_OF( array )  (sizeof array / sizeof array[ 0 ])

typedef uint32_t bits_t;  // uint8_t & uint16_t generate slightly larger code from GCC

char * parse( char ** ppLine );
int    search( char const * pKey,    char const * const * pTable, size_t       count );
int    search( uint16_t     key,     uint16_t     const * pTable, size_t       count );
int    search( double       key,     double       const * pTable, size_t       count,   double precision );
int    search( uint32_t     hz,      uint32_t     const * pTable, size_t       count );
bool   escape( char       * pExport, char         const * pName,  size_t       count );
void   strpad( char       * pTarget, size_t               count,  char const * pSource, char   pad );

union Endian16 {
  uint16_t value;
  char     bytes[ sizeof( uint16_t ) ];
  struct {
    char LOW2HI_ORDER( lowest,  // in terms of numerical significance
                       highest );
  } byte;
};

union Endian32 {
  uint32_t value;
  char     bytes[ sizeof( uint32_t ) ];
  struct {
    char LOW2HI_ORDER( lowest,  // in terms of numerical significance
         LOW2HI_ORDER( lower,
         LOW2HI_ORDER( higher,
                       highest )));
  } byte;
};

template< int BITS >
class FrequencyBE {       // big-endian target
  uint8_t HI2LOW_ORDER(         : CHAR_BIT - 1 - (BITS - 1) % CHAR_BIT,
                        hiOrder :            1 + (BITS - 1) % CHAR_BIT );
  uint8_t               lowOrder[                (BITS - 1) / CHAR_BIT ];
 public:
  uint32_t hz( uint32_t divisor = 1 ) const {
    assert( BITS <= 32 );  // Generates string literal but no code if true
    uint32_t result = hiOrder;
    for ( size_t index = 0; index < (BITS - 1) / CHAR_BIT;  index++ ) {
      result = result << CHAR_BIT | lowOrder[ index ];
    }
    return result * divisor;
  }
  void set( uint32_t hz, uint32_t divisor = 1 ) {
    assert( BITS <= 32 );  // Generates string literal but no code if true
    hz /= divisor;

    for ( size_t index = (BITS - 1) / CHAR_BIT;  index > 0;  hz >>= CHAR_BIT  ) {
      lowOrder[ --index ] = hz;
    }
    hiOrder = hz;
  }
};

template< int BITS >
class FrequencyLE {       // little-endian target
  uint8_t               lowOrder[                (BITS - 1) / CHAR_BIT ];
  uint8_t LOW2HI_ORDER( hiOrder :            1 + (BITS - 1) % CHAR_BIT,
                                : CHAR_BIT - 1 - (BITS - 1) % CHAR_BIT );
 public:
  uint32_t hz( uint32_t divisor = 1 ) const {
    assert( BITS <= 32 );  // Generates string literal but no code if true
    uint32_t result = hiOrder;
    for ( size_t index = (BITS - 1) / CHAR_BIT;  index-- > 0;  ) {
      result = result << CHAR_BIT | lowOrder[ index ];
    }
    return result * divisor;
  }
  void set( uint32_t hz, uint32_t divisor = 1 ) {
    assert( BITS <= 32 );  // Generates string literal but no code if true
    hz /= divisor;

    for ( size_t index = 0;  index < (BITS - 1) / CHAR_BIT;  hz >>= CHAR_BIT  ) {
      lowOrder[ index++ ] = hz;
    }
    hiOrder = hz;
  }
};

template< int CHARS, int BITS, char OFFSET >
class PackedString {
 private:
  char bytes[ (BITS * CHARS - 1) / CHAR_BIT + 1 ];

 public:
  PackedString & operator=( char const * string ) {
    assert( BITS * CHARS <= 64 );  // Generates string literal but no code if true
    uint64_t bits = (uint64_t)(bytes[ 0 ] >> sizeof bytes * CHAR_BIT % BITS);
    size_t   index;
    for ( index = 0;  index < CHARS;  index++ ) {
      bits <<= BITS;
      bits  |= ((*string ? *string++ : ' ') - OFFSET) & ((1 << BITS) - 1);
    }
    for ( index = sizeof bytes;  index-- > 0;  bits >>= CHAR_BIT ) {
      bytes[ index ] = bits;
    }
    return *this;
  }
  void unpack( char * string ) const {
    assert( BITS * CHARS <= 64 );  // Generates string literal but no code if true
    uint64_t bits = 0;
    size_t   index;
    for ( index = 0;  index < sizeof bytes;  index++ ) {
      bits <<= CHAR_BIT;
      bits  |= bytes[ index ];
    }
    for ( index = CHARS;  index-- > 0;  bits >>= BITS ) {
      string[ index ] = (bits & ((1 << BITS) - 1)) + OFFSET;
    }
    string[ CHARS ] = 0;
  }
};

class Radio;
typedef bool (Radio::*getField_t)( size_t index, char       * pExport ) const;
typedef bool (Radio::*setField_t)( size_t index, char const * pImport );

struct CsvField {
  char const * const fieldName;
  getField_t   const getField;
  setField_t   const setField;
};

class Radio {
 private:
  char const * const pHeader;

  Radio(             void              );  // Intentionally not implemented
  Radio(             Radio const & rhs );  // Intentionally not implemented
  Radio & operator=( Radio const & rhs );  // Intentionally not implemented

 protected:
  friend class FrequencySetBE_18;
  static double       const ctcssCodes[ 50 ];
  static uint16_t     const dcsCodes[  104 ];
  static uint32_t     const divisorsX3[  5 ];
  static char const * const dcsReverses[ 4 ];
  static char const * const skipModes[   3 ];
  static char const * const splits[      4 ];

  size_t              const size;
  uint8_t           * const pData;

  virtual uint8_t              getBit(                                size_t index               ) const {
    return 1 <<   index % CHAR_BIT;
  }
  virtual bool                 getBit(         uint8_t const * pBit,  size_t index               ) const {
    return (pBit[ index / CHAR_BIT ] & getBit( index )) != 0;
  }
  virtual bool                 setBit(         uint8_t       * pBit,  size_t index, bool value   ) const {
    if ( value ) {
      pBit[ index / CHAR_BIT ] |=      getBit( index );
    } else {
      pBit[ index / CHAR_BIT ] &=     ~getBit( index );
    }
    return true;
  }

  virtual char const         * getComment(                            char             * pExport ) const = 0;
  virtual char const         * setComment(                            char const       * pImport )       = 0;
  virtual size_t               getCount(       void                                              ) const = 0;
  virtual size_t               getOffset(      void                                              ) const {
    return 0;
  }

  virtual bool                 getValid(       size_t          index                             ) const = 0;
  virtual void                 setValid(       size_t          index, bool               value   )       = 0;
  virtual bool                _getValid(       size_t          index, char             * pExport ) const {
    return getValid( index );
  }
  virtual bool                _setValid(       size_t          index, char const       * pImport ) {
    if ( index >= getCount() ) {
      return false;
    }
    setValid( index, pImport != 0  &&  *pImport != 0 );
    return true;
  }

  virtual uint32_t             getRxFreq(      size_t          index                             ) const = 0;
  virtual bool                 setRxFreq(      size_t          index, uint32_t           value   )       = 0;
  virtual bool                _getRxFreq(      size_t          index, char             * pExport ) const {
    sprintf( pExport, "%.6f", getRxFreq( index ) / 1000000.0 );
    return true;
  }
  virtual bool                _setRxFreq(      size_t          index, char const       * pImport ) {
    char * pTemp;
    return setRxFreq( index, (uint32_t)(0.5 + 1000000.0 * strtod( pImport, & pTemp ) ) )  &&  *pTemp == 0;
  }

  virtual uint32_t             getTxFreq(      size_t          index                             ) const {
    assert( false );
    return 0;
  }
  virtual bool                 setTxFreq(      size_t          index, uint32_t           value   )       {
    assert( false );
    return 0;
  }
  virtual bool                _getTxFreq(      size_t          index, char             * pExport ) const {
    sprintf( pExport, "%.6f", getTxFreq( index ) / 1000000.0 );
    return true;
  }
  virtual bool                _setTxFreq(      size_t          index, char const       * pImport )       {
    char * pTemp;
    return setTxFreq( index, (uint32_t)(0.5 + 1000000.0 * strtod( pImport, & pTemp ) ) )  &&  *pTemp == 0;
  }

  virtual bits_t               getSplit(       size_t          index                             ) const = 0;
  virtual bool                 setSplit(       size_t          index, bits_t             value   )       = 0;
  virtual char const * const * getSplits(      size_t        * count                             ) const {
    *count = COUNT_OF( splits );
    return splits;
  }
  virtual bool                _getSplit(       size_t          index, char             * pExport ) const {
    size_t               count;
    char const * const * splits = getSplits( & count );
    size_t               value  = getSplit( index );
    if ( value >= count ) {
      sprintf( pExport, "%d", (int)value );
      return false;
    }
    strcpy( pExport, splits[ value ] );
    return true;
  }
  virtual bool                _setSplit(       size_t          index, char const       * pImport )       {
    size_t       count;
    char const * const * splits = getSplits( & count );
    int                  result = search( pImport, splits, count );
    if ( result < 0 ) {
      return false;
    }
    return setSplit(  index, result );
  }

  virtual uint32_t             getTxOffset(    size_t          index                             ) const {
    assert( false );
    return 0;
  }
  virtual bool                 setTxOffset(    size_t          index, uint32_t           value   )       {
    assert( false );
    return 0;
  }
  virtual bool                _getTxOffset(    size_t          index, char             * pExport ) const {
    sprintf( pExport, "%.6f", getTxOffset( index ) / 1000000.0 );
    return true;
  }
  virtual bool                _setTxOffset(    size_t          index, char const       * pImport ) {
    char * pTemp;
    return setTxOffset( index, (uint32_t)(0.5 + 1000000.0 * strtod( pImport, & pTemp ) ) )  &&  *pTemp == 0;
  }

  virtual bits_t               getRxStep(      size_t          index                             ) const = 0;
  virtual bool                 setRxStep(      size_t          index, bits_t             value   )       = 0;
  virtual double const       * getTuneSteps(   size_t        * count                             ) const = 0;
  virtual bool                _getRxStep(      bits_t          index, char             * pExport ) const {
    size_t         count;
    double const * tuneSteps = getTuneSteps( & count );
    size_t         value     = getRxStep( index );
    if ( value >= count ) {
      sprintf( pExport, "%d", (int)value );
      return false;
    }
    sprintf( pExport, "%gkHz", tuneSteps[ value ] );
    return true;
  }
  virtual bool                _setRxStep(      size_t          index, char const       * pImport )       {
    size_t         count;
    double const * tuneSteps = getTuneSteps( & count );
    char         * pTemp;
    int            result    = search( strtod( pImport, & pTemp ), tuneSteps, count, 0.005 );
    if ( result < 0 ) {
      return false;
    }
    return setRxStep( index, result );
  }

  virtual bits_t               getTxStep(         size_t                      index  ) const {
    assert( false );
    return 0;
  }
  virtual bool                 setTxStep(         size_t   index, bits_t       value   ) {
    assert( false );
    return 0;
  }
  virtual bool                _getTxStep(      bits_t          index, char             * pExport ) const {
    size_t         count;
    double const * tuneSteps = getTuneSteps( & count );
    size_t         value     = getTxStep( index );
    if ( value >= count ) {
      sprintf( pExport, "%d", (int)value );
      return false;
    }
    sprintf( pExport, "%gkHz", tuneSteps[ value ] );
    return true;
  }
  virtual bool                _setTxStep(      size_t          index, char const       * pImport )       {
    size_t         count;
    double const * tuneSteps = getTuneSteps( & count );
    char         * pTemp;
    int            result    = search( strtod( pImport, & pTemp ), tuneSteps, count, 0.005 );
    if ( result < 0 ) {
      return false;
    }
    return setTxStep( index, result );
  }

  virtual bits_t               getModulation(  size_t          index                             ) const = 0;
  virtual bool                 setModulation(  size_t          index, bits_t             value   )       = 0;
  virtual char const * const * getModulations( size_t        * count                             ) const = 0;
  virtual bool                _getModulation(  size_t          index, char             * pExport ) const {
    size_t               count;
    char const * const * modulations = getModulations( & count );
    size_t               value       = getModulation( index );
    if ( value >= count ) {
      sprintf( pExport, "%d", (int)value );
      return false;
    }
    strcpy( pExport, modulations[ value ] );
    return true;
  }
  virtual bool                _setModulation(  size_t          index, char const       * pImport )       {
    size_t               count;
    char const * const * modulations = getModulations( & count );
    int                  result      = search( pImport, modulations, count );
    if ( result < 0 ) {
      return false;
    }
    return setModulation( index, result );
  }

  virtual char const         * getName(        size_t          index, size_t           * pSize   ) const = 0;
  virtual bool                 setName(        size_t          index, char const       * pvalue  )       = 0;
  virtual bool                _getName(        size_t          index, char             * pExport ) const {
    size_t       size; 
    char const * pName = getName( index, & size );
    return escape( pExport, pName, size );
  }
  virtual bool                _setName(        size_t          index, char const       * pImport )       {
    char work[ 256 ];
    strcpy( work, & pImport[ strspn( pImport, "\"" ) ] );
    work[ strcspn( work, "\"" ) ] = 0;
    return setName( index, work );
  }

  virtual bits_t               getFilter(      size_t          index                             ) const {
    assert( false );
    return 0;
  }
  virtual bool                 setFilter(      size_t          index, bits_t             value   )       {
    assert( false );
    return 0;
  }
  virtual bool                _getFilter(      size_t          index, char             * pExport ) const {
    sprintf( pExport, "%d", (int)getFilter( index ) );
    return true;
  }
  virtual bool                _setFilter(      size_t          index, char const       * pImport ) {
    char * pTemp;
    int result = strtoul( pImport, & pTemp, 10 );
    if ( *pTemp != 0  ||  result < 1  ||  result > 3 ) {
      return false;
    }
    return setFilter( index, result );
  }

  virtual bits_t               getFmSquelch(   size_t          index                             ) const = 0;
  virtual bool                 setFmSquelch(   size_t          index, bits_t             value   )       = 0;
  virtual char const * const * getFmSquelches( size_t        * count                             ) const = 0;
  virtual bool                _getFmSquelch(   size_t          index, char             * pExport ) const {
    size_t               count;
    char const * const * fmSquelches = getFmSquelches( & count );
    size_t               value       = getFmSquelch( index );
    if ( value >= count ) {
      sprintf( pExport, "%d", (int)value );
      return false;
    }
    strcpy( pExport, fmSquelches[ value ] );
    return true;
  }
  virtual bool                _setFmSquelch(   size_t          index, char const       * pImport )       {
    size_t               count;
    char const * const * fmSquelches = getFmSquelches( & count );
    int                  result      = search( pImport, fmSquelches, count );
    if ( result < 0 ) {
      return false;
    }
    return setFmSquelch( index, result );
  }

  virtual bits_t               getCtcssEncode( size_t          index                             ) const = 0;
  virtual bool                 setCtcssEncode( size_t          index, bits_t             value   )       = 0;
  virtual double const       * getCtcssCodes(  size_t        * count                             ) const {
    *count = COUNT_OF( ctcssCodes );
    return ctcssCodes;
  }
  virtual bool                _getCtcssEncode( size_t          index, char             * pExport ) const {
    size_t         count;
    double const * ctcssCodes = getCtcssCodes( & count );
    size_t         value      = getCtcssEncode( index );
    if ( value >= count ) {
      sprintf( pExport, "%d", (int)value );
      return false;
    }
    sprintf( pExport, "%.1fHz", ctcssCodes[ value ] );
    return true;
  }
  virtual bool                _setCtcssEncode( size_t          index, char const       * pImport )       {
    size_t         count;
    double const * ctcssCodes = getCtcssCodes( & count );
    char         * pTemp;
    int            result     = search( strtod( pImport, & pTemp ), ctcssCodes, count, 0.05 );
    if ( result < 0 ) {
      return false;
    }
    return setCtcssEncode( index, result );
  }

  virtual bits_t               getCtcssDecode( size_t          index                             ) const = 0;
  virtual bool                 setCtcssDecode( size_t          index, bits_t             value   )       = 0;
  virtual bool                _getCtcssDecode( size_t          index, char             * pExport ) const {
    size_t         count;
    double const * ctcssCodes = getCtcssCodes( & count );
    size_t         value      = getCtcssDecode( index );
    if ( value >= count ) {
      sprintf( pExport, "%d", (int)value );
      return false;
    }
    sprintf( pExport, "%.1fHz", ctcssCodes[ value ] );
    return true;
  }
  virtual bool                _setCtcssDecode( size_t          index, char const       * pImport )       {
    size_t         count;
    double const * ctcssCodes = getCtcssCodes( & count );
    char         * pTemp;
    int            result     = search( strtod( pImport, & pTemp ), ctcssCodes, count, 0.05 );
    if ( result < 0 ) {
      return false;
    }
    return setCtcssDecode( index, result );
  }

  virtual bits_t               getDcsCode(     size_t          index                             ) const = 0;
  virtual bool                 setDcsCode(     size_t          index, bits_t             value   )       = 0;
  virtual uint16_t const     * getDcsCodes(    size_t        * count                             ) const {
    *count = COUNT_OF( dcsCodes );
    return dcsCodes;
  }
  virtual bool                _getDcsCode(     size_t          index, char             * pExport ) const {
    size_t           count;
    uint16_t const * dcsCodes = getDcsCodes( & count );
    size_t           value    = getDcsCode( index );
    if ( value >= count ) {
      sprintf( pExport, "%d", (int)value );
      return false;
    }
    sprintf( pExport, "%03d", dcsCodes[ value ] );
    return true;
  }
  virtual bool                _setDcsCode(     size_t          index, char const       * pImport )       {
    size_t           count;
    uint16_t const * dcsCodes = getDcsCodes( & count );
    char           * pTemp;
    int              result   = search( strtoul( pImport, & pTemp, 10 ), dcsCodes, count );
    if ( *pTemp != 0  ||  result < 0 ) {
      return false;
    }
    return setDcsCode( index, result );
  }

  virtual bits_t               getDcsReverse(  size_t          index                             ) const = 0;
  virtual bool                 setDcsReverse(  size_t          index, bits_t             value   )       = 0;
  virtual char const * const * getDcsReverses( size_t        * count                             ) const {
    *count = COUNT_OF( dcsReverses );
    return dcsReverses;
  }
  virtual bool                _getDcsReverse(  size_t          index, char             * pExport ) const {
    size_t               count;
    char const * const * dcsReverses = getDcsReverses( & count );
    size_t               value       = getDcsReverse( index );
    if ( value >= count ) {
      sprintf( pExport, "%d", (int)value );
      return false;
    }
    strcpy( pExport, dcsReverses[ value ] );
    return true;
  }
  virtual bool                _setDcsReverse(  size_t          index, char const       * pImport )       {
    size_t               count;
    char const * const * dcsReverses = getDcsReverses( & count );
    int                  result      = search( pImport, dcsReverses, count );
    if ( result < 0 ) {
      return false;
    }
    return setDcsReverse( index, result );
  }


  virtual CsvField const * csvHeader( void ) const = 0;

  Radio(  char const * pHeader, uint8_t const * pData, size_t size )
      : pHeader( strdup( pHeader ) ), size( size ),  pData( (uint8_t *)memcpy( malloc( size ), pData, size ) ) {}

 public:
  virtual         ~Radio( void ) {
    free( (void *)pHeader );
    free( (void *)pData   );
  }
  static  Radio * create( FILE * pFile, bool isBinary                       );
  virtual void    save(   FILE * pFile, bool isBinary, char const * comment );
  virtual void    dump(   FILE * pFile                                      ) const;
  virtual bool    load(   FILE * pFile                                      );
};

class FrequencySetBE_18 {
  union {
    FrequencyBE< 18 > rxFreq;
    struct {
      uint8_t HI2LOW_ORDER( rx : 3,
              HI2LOW_ORDER( tx : 3,
                               : 2 ));
    } divisor;
  };
  FrequencyBE< 16 > txOffset;
 public:
  uint32_t rxHz( void ) const {
    return rxFreq.hz( Radio::divisorsX3[ divisor.rx ] ) / 3;
  }
  uint32_t txHz( void ) const {
    return txOffset.hz( Radio::divisorsX3[ divisor.tx ] ) / 3;
  }
  bool rxSet( uint32_t hz ) {
    int result = search( hz * 3, Radio::divisorsX3, COUNT_OF( Radio::divisorsX3 ) );
    divisor.rx = result >= 0 ? result : 2;
    rxFreq.set( hz * 3, Radio::divisorsX3[ divisor.rx ] );
    return result >= 0;
  }
  bool txSet( uint32_t hz ) {
    int result = search( hz * 3, Radio::divisorsX3, COUNT_OF( Radio::divisorsX3 ) );
    divisor.tx = result >= 0 ? result : 2;
    txOffset.set( hz * 3, Radio::divisorsX3[ divisor.tx ] );
    return result >= 0;
  }
  void debug( char * pWork ) const {
    sprintf( pWork, "%d %d %05X", divisor.rx, divisor.tx, rxFreq.hz() );
  }
};
