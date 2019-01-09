//==================================================================================================
//  Ic7300.cpp is the Icom IC-7300 radio class file for Radio2csv.
//
//  Author:  Copyright (c) 2016-2017 by Dean K. Gibson.
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

#include "Radio.hpp"

static char const * const modulations[] = { "LSB", "LSB-D",    "USB", "USB-D", "CW",  "?2?", "CW-R",   "?3", 
                                           "RTTY",   "?4?", "RTTY-R",   "?5?", "AM", "AM-D",   "FM", "FM-D" };
static char const * const fmSquelches[] = { "OFF",  "TONE",   "TSQL" };

struct Ic7300Message {
  enum {
    MESSAGE_LENGTH = 70
  };

  char text[ MESSAGE_LENGTH ];
};

struct Ic7300ScopeEdge {  // little-endian target
  enum {
    SCOPE_EDGES = 3
  };

  struct _scopeEdge {
    FrequencyLE< 32 > lower,
                      upper;
  } edges[ SCOPE_EDGES ];
};

struct Ic7300Setting {    // little-endian target
  FrequencyLE< 32 >     freq;
  uint8_t LOW2HI_ORDER( filter     : 4,  //0-2 => 1-3
                        modulation : 4 );
  uint8_t LOW2HI_ORDER( data       : 2,
          LOW2HI_ORDER( fmSquelch  : 2,
                                   : 4 ));
  uint8_t               ctcssEncode;
  uint8_t               ctcssDecode;
};

struct Ic7300Channel {    // little-endian target
  uint8_t               zero1[ 2 ];
  uint8_t LOW2HI_ORDER(            : 1,
          LOW2HI_ORDER( split      : 1,
                                   : 6 ));
  uint8_t               zero2[ 3 ];
  Ic7300Setting         rx,
                        tx;
  char                  name[ 10 ];
};

struct Ic7300Memory {
  enum {
    MESSAGE_COUNT  =  8,
    BAND_STACKS    =  3,
    MPADS          = 10 + 1,
    CHANNELS       = 99
  };

  // { 0x06, 0x0D, 0x66, 0x62, 0x63, 0x60, 0x75, 0x03,
  //   0x66, 0x7B, 0x61, 0x64, 0x78, 0x65, 0x65, 0x65 };     //  "..fbc`u.f{adxeee"
  uint8_t          header[    16 ];                          // 0x0000
  uint8_t          lengthLE[   2 ];                          // 0x0010
  Ic7300Channel    channel[      CHANNELS ],                 // 0x0012
                   lowerScanEdge,                            // 0x0C72
                   upperScanEdge;                            // 0x0C92
  uint8_t          dummy1[     2 ];                          // 0x0CB2
  uint8_t          isValid[      CHANNELS / CHAR_BIT + 1 ];  // 0x0CB4
  uint8_t          scan[     2 * CHANNELS / CHAR_BIT + 2 ];  // 0x0CC1
  uint8_t          dummy2;                                   // 0x0CDB
  Ic7300Setting    stack160m[    BAND_STACKS ],              // 0x0CDC
                   stack80m[     BAND_STACKS ],              // 0x0CF4
                   stack40m[     BAND_STACKS ],              // 0x0D0C
                   stack30m[     BAND_STACKS ],              // 0x0D24
                   stack20m[     BAND_STACKS ],              // 0x0D3C
                   stack17m[     BAND_STACKS ],              // 0x0D54
                   stack15m[     BAND_STACKS ],              // 0x0D6C
                   stack12m[     BAND_STACKS ],              // 0x0D84
                   stack10m[     BAND_STACKS ],              // 0x0D9C
                   stack6m[      BAND_STACKS ],              // 0x0DB4
                   stackGen[     BAND_STACKS ];              // 0x0DCC
  uint8_t          dummy3[    20 ];                          // 0x0DE4
  Ic7300Setting    mpad[ MPADS ];                            // 0x0DF8
  Ic7300Message    cwMessages[   MESSAGE_COUNT ],            // 0x0E50
                   rttyMessages[ MESSAGE_COUNT ];            // 0x1080
  uint8_t          dummy4[ 58*16 ];                          // 0x12B0
  char             callsign[  10 ];                          // 0x1650
  uint8_t          dummy5[    26 ];                          // 0x165A
  Ic7300ScopeEdge  scopeEdge03_16,                           // 0x1674
                   scopeEdge16_20,                           // 0x168C
                   scopeEdge02_06,                           // 0x16A4
                   scopeEdge06_08,                           // 0x16BC
                   scopeEdge08_11,                           // 0x16D4
                   scopeEdge11_15,                           // 0x16EC
                   scopeEdge15_20,                           // 0x1704
                   scopeEdge20_22,                           // 0x171C
                   scopeEdge22_26,                           // 0x1734
                   scopeEdge26_30,                           // 0x174C
                   scopeEdge30_45,                           // 0x1764
                   scopeEdge45_60,                           // 0x177C
                   scopeEdge60_75;                           // 0x1794
  uint8_t          dummy6[   724 ];                          // 0x17AC
  uint8_t          checkSumLE[ 2 ];                          // 0x1A80
  uint8_t          zero9[      2 ];                          // 0x1A82
};                                                           // 0x1A84

class Ic7300 : public Radio {
  Ic7300Memory * pMemory;

  Ic7300(             void            );     // Intentionally not implemented
  Ic7300(             Ic7300 const & rhs );  // Intentionally not implemented
  Ic7300 & operator=( Ic7300 const & rhs );  // Intentionally not implemented

 protected:
  static CsvField const csvFields[];

  virtual CsvField const * csvHeader( void ) const {
    return csvFields;
  }

  virtual char const         * getComment(                      char       * pExport ) const {
    *pExport = 0;
    return "Icom IC-7300";
  }
  virtual char const         * setComment(                      char const * pImport )       {
    return "Icom IC-7300";
  }
  virtual size_t               getCount(       void                                  ) const {
    return Ic7300Memory::CHANNELS;
  }
  virtual size_t               getOffset(      void                                  ) const {
    return 1;
  }

  virtual bits_t               getSplit(       size_t   index               ) const { assert( false ); return 0; }
  virtual bool                 setSplit(       size_t   index, bits_t value )       { assert( false ); return 0; }
  virtual bits_t               getRxStep(      size_t   index               ) const { assert( false ); return 0; }
  virtual bool                 setRxStep(      size_t   index, bits_t value )       { assert( false ); return 0; }
  virtual double const       * getTuneSteps(   size_t * count               ) const { assert( false ); return 0; }
  virtual bits_t               getDcsCode(     size_t   index               ) const { assert( false ); return 0; }
  virtual bool                 setDcsCode(     size_t   index, bits_t value )       { assert( false ); return 0; }
  virtual bits_t               getDcsReverse(  size_t   index               ) const { assert( false ); return 0; }
  virtual bool                 setDcsReverse(  size_t   index, bits_t value )       { assert( false ); return 0; }

  virtual bool                 getValid(       size_t   index                       ) const {
    return (pMemory->isValid[ index / CHAR_BIT ] & 1 << index % CHAR_BIT) == 0;
  }
  virtual void                 setValid(       size_t   index, bool         value   ) {
    if ( value ) {
      pMemory->isValid[ index / CHAR_BIT ] &= ~(1 <<     index % CHAR_BIT);
    } else {
      pMemory->isValid[ index / CHAR_BIT ] |=   1 <<     index % CHAR_BIT;
    }
    memcpy( & pMemory->channel[ index ], & pMemory->lowerScanEdge, sizeof pMemory->channel[ 0 ] );
    setName( index, "" );
  }

  virtual char const         * getName(        size_t   index, size_t     * pSize   ) const {
    *pSize = sizeof pMemory->channel[ 0 ].name;
    return pMemory->channel[ index ].name;
  }
  virtual bool                 setName(        size_t   index, char const * pValue  ) {
    strpad( pMemory->channel[ index ].name, sizeof pMemory->channel[ 0 ].name, pValue, ' ');
    return true;
  }

  virtual uint32_t             getRxFreq(      size_t   index                       ) const {
    return pMemory->channel[ index ].rx.freq.hz();
  }
  virtual bool                 setRxFreq(      size_t   index, uint32_t     value   ) {
    pMemory->channel[ index ].rx.freq.set( value );
    return true;
  }

  virtual uint32_t             getTxFreq(      size_t   index                       ) const {
    return pMemory->channel[ index ].split != 0
           ? pMemory->channel[ index ].tx.freq.hz() : 0;
  }
  virtual bool                 setTxFreq(      size_t   index, uint32_t     value   ) {
    pMemory->channel[ index ].tx.freq.set( value );
    return true;
  }

  virtual bits_t               getModulation(  size_t   index                       ) const {
    return (pMemory->channel[ index ].rx.modulation << 1) + pMemory->channel[ index ].rx.data;
  }
  virtual bool                 setModulation(  size_t   index, bits_t        value  ) {
    pMemory->channel[ index ].rx.modulation = value >> 1;
    pMemory->channel[ index ].tx.modulation = value >> 1;
    pMemory->channel[ index ].rx.data       = value & 1;
    pMemory->channel[ index ].tx.data       = value & 1;
    return true;
  }
  virtual char const * const * getModulations( size_t * count                       ) const {
    *count = COUNT_OF( ::modulations );
    return ::modulations;
  }

  virtual bits_t               getFilter(      size_t   index                       ) const {
    return pMemory->channel[ index ].rx.filter + 1;
  }
  virtual bool                 setFilter(      size_t   index, bits_t       value   ) {
    pMemory->channel[ index ].rx.filter = value - 1;
    pMemory->channel[ index ].tx.filter = value - 1;
    return true;
  }

  virtual bits_t               getScan(        size_t   index                       ) const {
    return pMemory->scan[ 2 * index / CHAR_BIT ] >> 2 * index % CHAR_BIT & 3;
  }
  virtual bool                 setScan(        size_t   index, bits_t       value   ) {
    pMemory->scan[ 2 * index / CHAR_BIT ] &= ~(  3 << 2 * index % CHAR_BIT);
    pMemory->scan[ 2 * index / CHAR_BIT ] |= value << 2 * index % CHAR_BIT;
    return true;
  }
  virtual bool                _getScan(        size_t   index, char       * pExport ) const {
    size_t value = getScan( index );
    sprintf( pExport, "%d", (int)value );
    return value < 4;
  }
  virtual bool                _setScan(        size_t   index, char const * pImport ) {
    char * pTemp;
    int result = strtoul( pImport, & pTemp, 10 );
    if ( *pTemp != 0  ||  result < 0  ||  result > 3 ) {
      return false;
    }
    return setScan( index, result );
  }

  virtual bits_t               getFmSquelch(   size_t   index                       ) const {
    return pMemory->channel[ index ].rx.fmSquelch;
  }
  virtual bool                 setFmSquelch(   size_t   index, bits_t        value  ) {
    pMemory->channel[ index ].rx.fmSquelch = value;
    pMemory->channel[ index ].tx.fmSquelch = value;
    return true;
  }
  virtual char const * const * getFmSquelches( size_t * count                       ) const {
    *count = COUNT_OF( ::fmSquelches );
    return ::fmSquelches;
  }

  virtual bits_t               getCtcssEncode( size_t   index                       ) const {
    return pMemory->channel[ index ].rx.ctcssEncode;
  }
  virtual bool                 setCtcssEncode( size_t   index, bits_t       value   ) {
    pMemory->channel[ index ].rx.ctcssEncode = value;
    pMemory->channel[ index ].tx.ctcssEncode = value;
    return true;
  }

  virtual bits_t               getCtcssDecode( size_t   index                       ) const {
    return pMemory->channel[ index ].rx.ctcssDecode;
  }
  virtual bool                 setCtcssDecode( size_t   index, bits_t       value   ) {
    pMemory->channel[ index ].rx.ctcssDecode = value;
    pMemory->channel[ index ].tx.ctcssDecode = value;
    return true;
  }

  virtual void save( FILE * pFile, bool isBinary, char const * pComment ) {
    char const * pModel = setComment( pComment );
    Endian16 checkSum = { 0 };
    for ( uint8_t * pSum  = pMemory->lengthLE + sizeof pMemory->lengthLE;
                    pSum  < pMemory->checkSumLE;
          checkSum.value  -= *pSum++ );
    pMemory->checkSumLE[ 0 ] = checkSum.byte.lowest;
    pMemory->checkSumLE[ 1 ] = checkSum.byte.highest;
    fwrite( pData, 1, size, pFile );
    fprintf( stderr, "--- %s updated ---\n", pModel );
  }

 public:
  Ic7300( char    const * pHeader,
          uint8_t const * pData,
          size_t          size )
      : Radio( pHeader, pData, size ), pMemory( (Ic7300Memory *)Radio::pData ) {
    assert( sizeof( Ic7300Channel ) == 32 );
    Endian16  checkSum;
    checkSum.byte.lowest  = pMemory->checkSumLE[ 0 ];
    checkSum.byte.highest = pMemory->checkSumLE[ 1 ];
    for ( uint8_t * pSum =  pMemory->lengthLE + sizeof pMemory->lengthLE;
                    pSum <  pMemory->checkSumLE;
          checkSum.value += *pSum++ );
    assert( checkSum.value == 0 );
  }
  virtual ~Ic7300( void ) {}
};

CsvField const Ic7300::csvFields[]
    = { { "CH No",        reinterpret_cast< getField_t >( & Ic7300::_getValid        ),
                          reinterpret_cast< setField_t >( & Ic7300::_setValid        ) },
        { "Name",         reinterpret_cast< getField_t >( & Ic7300::_getName         ),
                          reinterpret_cast< setField_t >( & Ic7300::_setName         ) },
        { "Scan",         reinterpret_cast< getField_t >( & Ic7300::_getScan         ),
                          reinterpret_cast< setField_t >( & Ic7300::_setScan         ) },
        { "Rx Freq",      reinterpret_cast< getField_t >( & Ic7300::_getRxFreq       ),
                          reinterpret_cast< setField_t >( & Ic7300::_setRxFreq       ) },
        { "Modulation",   reinterpret_cast< getField_t >( & Ic7300::_getModulation   ),
                          reinterpret_cast< setField_t >( & Ic7300::_setModulation   ) },
        { "Filter",       reinterpret_cast< getField_t >( & Ic7300::_getFilter       ),
                          reinterpret_cast< setField_t >( & Ic7300::_setFilter       ) },
        { "CTCSS Mode",   reinterpret_cast< getField_t >( & Ic7300::_getFmSquelch    ),
                          reinterpret_cast< setField_t >( & Ic7300::_setFmSquelch    ) },
        { "Tone Encode",  reinterpret_cast< getField_t >( & Ic7300::_getCtcssEncode  ),
                          reinterpret_cast< setField_t >( & Ic7300::_setCtcssEncode  ) },
        { "TSQL Decode",  reinterpret_cast< getField_t >( & Ic7300::_getCtcssDecode  ),
                          reinterpret_cast< setField_t >( & Ic7300::_setCtcssDecode  ) },
        { "Tx Freq",      reinterpret_cast< getField_t >( & Ic7300::_getTxFreq       ),
                          reinterpret_cast< setField_t >( & Ic7300::_setTxFreq       ) },
        { 0, 0, 0 } };


Radio * newIc7300( char    const * pHeader,
                   uint8_t const * pData,
                   size_t          size ) {
  Endian16  length;
  length.byte.lowest  = ((Ic7300Memory *)pData)->lengthLE[ 0 ];
  length.byte.highest = ((Ic7300Memory *)pData)->lengthLE[ 1 ];
  return size == sizeof( Ic7300Memory )  &&  size == length.value
         ? new Ic7300( pHeader, pData, size ) : 0;
}
