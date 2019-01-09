//==================================================================================================
//  Id800.cpp is the Icom ID-800H radio class file for Radio2csv.
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

#include "Dstar.hpp"

static char const * const powerLevels[] = { "High",  "Low",  "Mid"  };
static char const * const modulations[] = {   "FM", "FM-N",   "AM", "AM-N", "DV"  };
static char const * const fmSquelches[] = {  "OFF", "TONE", "TSQL", "DTCS" };
static double       const tuneSteps[]   = {    5.0,   10.0,   12.5,   15.0, 20.0,   25.0,   30.0,   50.0,
                                             100.0,  200.0,   6.25  };
static char         const booleans[]    = "FT";

union Id800Suffix {
  uint8_t bits;
  uint8_t HI2LOW_ORDER(         : 1,
          HI2LOW_ORDER( ignore  : 1,
          HI2LOW_ORDER( skipp   : 1,
          HI2LOW_ORDER( skip    : 1,
                        bank    : 4 ))));
};

typedef PackedString< 6, 6, ' ' > Id800Name;

struct Id800Channel {   // big-endian target
  FrequencyBE< 24 >     rxFreq;
  FrequencyBE< 16 >     txOffset;
  uint8_t HI2LOW_ORDER( powerLevel    : 2,
                        ctcssEncode   : 6 );
  uint8_t HI2LOW_ORDER( direction     : 2,
                        ctcssDecode   : 6 );
  uint8_t HI2LOW_ORDER(               : 1,
                        dcsCode       : 7 );
  uint8_t HI2LOW_ORDER( tuneStep      : 4,
                                      : 4 );
  uint8_t               unknown1;           // 0xE4
  uint8_t HI2LOW_ORDER( rxResolution  : 1,
          HI2LOW_ORDER( txResolution  : 1,
          HI2LOW_ORDER( unknown2      : 4,  // 0xC
                        fmSquelch     : 2 )));
  union {
    struct {
      uint8_t HI2LOW_ORDER( dcsReverse : 2,
              HI2LOW_ORDER( display     : 1,
                            dummy       : 5 ));
    }                   overlay;
    Id800Name          name;
  };
  uint8_t HI2LOW_ORDER(               : 1,
                        dvCsqlCode    : 7 );
  uint8_t HI2LOW_ORDER(               : 1,
          HI2LOW_ORDER( dvSquelch     : 2,
          HI2LOW_ORDER(               : 4,
                        useRpt        : 1 )));
  uint8_t HI2LOW_ORDER(               : 1,
                        yourCall      : 7 );
  uint8_t HI2LOW_ORDER(               : 2,
                        rpt1Call      : 6 );
  uint8_t HI2LOW_ORDER(               : 2,
                        rpt2Call      : 6 );
  uint8_t HI2LOW_ORDER( modulation    : 4,
                                      : 4 );
};

struct Id800Memory {
  enum {
    CHANNELS =  500,
    BANKS    =   10,
    EDGES    =    5
  };

  char          header1[ 16 ];        // 0x0000
  char          header2[ 16 ];        // 0x0010
  Id800Channel  channel[ CHANNELS ];  // 0x0020
  struct {
    Id800Channel  lower,
                  upper;
  }             scanEdge[ EDGES ];    // 0x2B18
  Id800Suffix   suffix[ CHANNELS ];   // 0x2BF4
  uint8_t       fill1[ 1080 ];        // 0x2DE8
  Routing       myCall[   6 ];        // 0x3220
  Routing       urCall[ 100 ];        // 0x3250 -- 100th item is never used
  Routing       rpCall[  54 ];        // 0x3570
  uint8_t       fill2[  392 ];        // 0x3720
  char          comment[ 16 ];        // 0x38A8
  uint8_t       fill3[ 1864 ];        // 0x38B8
};                                    // 0x4000

class Id800 : public Dstar {
  Id800Memory * pMemory;

  Id800(             void              );  // Intentionally not implemented
  Id800(             Id800 const & rhs );  // Intentionally not implemented
  Id800 & operator=( Id800 const & rhs );  // Intentionally not implemented

 protected:
  static CsvField     const csvFields[];

  virtual bits_t getBankChannel(  size_t   index                       ) const { assert( false ); return 0; }
  virtual bool   setBankChannel(  size_t   index, bits_t       value   )       { assert( false ); return 0; }

  virtual bool                 getIgnore(       size_t   index                       ) const {
    return pMemory->suffix[ index ].ignore;
  }
  virtual bool                 setIgnore(       size_t   index, bool         value   )       {
    pMemory->suffix[ index ].ignore = value ? 0 : 1;
    return true;
  }
  virtual bool                 getSkip(         size_t   index                       ) const {
    return pMemory->suffix[ index ].skip;
  }
  virtual bool                 setSkip(         size_t   index, bool         value   )       {
    pMemory->suffix[ index ].skip = value ? 1 : 0;
    return true;
  }
  virtual bool                 getSkipp(        size_t   index                       ) const {
    return pMemory->suffix[ index ].skipp;
  }
  virtual bool                 setSkipp(        size_t   index, bool         value   )       {
    pMemory->suffix[ index ].skipp = value ? 1 : 0;
    return true;
  }

  virtual char const         * getComment(                      char       * pExport ) const {
    memcpy( pExport, pMemory->comment, sizeof pMemory->comment );
    pExport[ sizeof pMemory->comment ] = 0;
    return "Icom ID-800H";
  }
  virtual char const         * setComment(                      char const * pImport )       {
    if ( pImport != 0 ) {
      strpad( pMemory->comment, sizeof pMemory->comment, pImport, ' ' );
    }
    return "Icom ID-800H";
  }
  virtual size_t               getCount(        void                               ) const {
    return Id800Memory::CHANNELS;
  }

  virtual CsvField const * csvHeader( void ) const {
    return csvFields;
  }

/*
  virtual bool                 getDisplay(       size_t           index  ) const;
  virtual bool                 setDisplay(       size_t           index,
                                         bool             value  );
    sprintf( pExport, ",%c", booleans[ getDisplay( index ) ? 1 : 0 ] );
    setDisplay(      index,            booleans[ 0 ] != toupper( * pExport )  );
*/

  virtual bool                 getValid(        size_t   index                       ) const {
    return !getIgnore( index );
  }
  virtual void                 setValid(        size_t   index, bool         value   ) {
    memset( & pMemory->channel[ index ], value ? 0 : -1, sizeof pMemory->channel[ 0 ] );
    pMemory->suffix[  index ].bits = 0;
    pMemory->channel[ index ].unknown1 = 0xE4;
    pMemory->channel[ index ].unknown2 = 0xC;
    setName(   index, "" );
    setIgnore( index, !value );
  }

  virtual uint32_t             getRxFreq(       size_t   index                       ) const {
    return pMemory->channel[ index ].rxFreq.hz( divisorsX3[ pMemory->channel[ index ].rxResolution ] ) / 3;
  }
  virtual bool                 setRxFreq(       size_t   index, uint32_t     value   ) {
    int    result     = search( value * 3, divisorsX3, COUNT_OF( divisorsX3 ) );
    size_t divisor = result >= 0 ? result : 0;
    pMemory->channel[ index ].rxResolution = divisor;
    pMemory->channel[ index ].rxFreq.set( value * 3, divisorsX3[ divisor ] );
    return result >= 0;
  }

  virtual bits_t               getSplit(        size_t   index                       ) const {
    size_t value = pMemory->channel[ index ].direction;
    return value ? value - 1 : 0;
  }
  virtual bool                 setSplit(        size_t   index, bits_t       value   ) {
    pMemory->channel[ index ].direction = value ? value + 1 : 0;
    return true;
  }

  virtual uint32_t             getTxOffset(     size_t   index                       ) const {
    return pMemory->channel[ index ].txOffset.hz( divisorsX3[ pMemory->channel[ index ].txResolution ] ) / 3;
  }
  virtual bool                 setTxOffset(     size_t   index, uint32_t     value   ) {
    int    result     = search( value * 3, divisorsX3, COUNT_OF( divisorsX3 ) );
    size_t divisor = result >= 0 ? result : 0;
    pMemory->channel[ index ].txResolution = divisor;
    pMemory->channel[ index ].txOffset.set( value * 3, divisorsX3[ divisor ] );
    return result >= 0;
  }

  virtual bits_t               getRxStep(       size_t   index                       ) const {
    return pMemory->channel[ index ].tuneStep;
  }
  virtual bool                 setRxStep(       size_t   index, bits_t       value   ) {
    pMemory->channel[ index ].tuneStep = value;
    return true;
  }
  virtual double const       * getTuneSteps(    size_t * count                       ) const {
    *count = COUNT_OF( ::tuneSteps );
    return ::tuneSteps;
  }

  virtual bits_t               getPowerLevel(   size_t   index                       ) const {
    return pMemory->channel[ index ].powerLevel;
  }
  virtual bool                 setPowerLevel(   size_t   index, bits_t       value   ) {
    pMemory->channel[ index ].powerLevel = value;
    return true;
  }
  virtual char const * const * getPowerLevels(  size_t * count                       ) const {
    *count = COUNT_OF( ::powerLevels );
    return ::powerLevels;
  }
  virtual bool                _getPowerLevel(   size_t   index, char       * pExport ) const {
    size_t               count;
    char const * const * powerLevels = getPowerLevels( & count );
    size_t               value       = getPowerLevel( index );
    if ( value >= count ) {
      sprintf( pExport, "%d", (int)value );
      return false;
    }
    strcpy( pExport, powerLevels[ value ] );
    return true;
  }
  virtual bool                _setPowerLevel(   size_t   index, char const * pImport ) {
    size_t               count;
    char const * const * powerLevels = getPowerLevels( & count );
    int                  result      = search( pImport, powerLevels, count );
    if ( result < 0 ) {
      return false;
    }
    return setPowerLevel( index, result );
  }

  virtual bits_t               getModulation(   size_t   index                       ) const {
    return pMemory->channel[ index ].modulation;
  }
  virtual bool                 setModulation(   size_t   index, bits_t       value   ) {
    pMemory->channel[ index ].modulation = value;
    return true;
  }
  virtual char const * const * getModulations(  size_t * count                       ) const {
    *count = COUNT_OF( ::modulations );
    return ::modulations;
  }

  virtual char const         * getName(         size_t   index, size_t     * pSize   ) const {
    assert( false );
    return 0;
  }
  virtual Id800Name const    * getName(         size_t   index                       ) const {
    return & pMemory->channel[ index ].name;
  }
  virtual bool                 setName(         size_t   index, char const * pValue  ) {
    pMemory->channel[ index ].name = pValue;
    return true;
  }
  virtual bool                _getName(         size_t   index, char       * pExport ) const {
    char work[ 256 ];
    getName( index )->unpack( work );
    return escape( pExport, work, sizeof work );
  }

  virtual bits_t               getSkipMode(     size_t   index                       ) const {
    return getSkipp( index ) ? 2 : getSkip( index ) ? 1 : 0;
  }
  virtual bool                 setSkipMode(     size_t   index, bits_t       value   ) {
    setSkipp( index, false );
    setSkip(  index, false );
    switch ( value ) {
      case 0:
        return true;
      case 2:
        setSkipp( index, true );
        // fall-thru
      case 1:
        setSkip(  index, true );
        return true;
    }
    return false;
  }

  virtual bits_t               getFmSquelch(    size_t   index                       ) const {
    return pMemory->channel[ index ].fmSquelch;
  }
  virtual bool                 setFmSquelch(    size_t   index, bits_t       value   ) {
    pMemory->channel[ index ].fmSquelch = value;
    return true;
  }
  virtual char const * const * getFmSquelches(  size_t * count                       ) const {
    *count = COUNT_OF( ::fmSquelches );
    return ::fmSquelches;
  }

  virtual bits_t               getCtcssEncode(  size_t   index                       ) const {
    return pMemory->channel[ index ].ctcssEncode;
  }
  virtual bool                 setCtcssEncode(  size_t   index, bits_t       value   ) {
    pMemory->channel[ index ].ctcssEncode = value;
    return true;
  }

  virtual bits_t               getCtcssDecode(  size_t   index                       ) const {
    return pMemory->channel[ index ].ctcssDecode;
  }
  virtual bool                 setCtcssDecode(  size_t   index, bits_t       value   ) {
    pMemory->channel[ index ].ctcssDecode = value;
    return true;
  }

  virtual bits_t               getDcsCode(      size_t   index                       ) const {
    return pMemory->channel[ index ].dcsCode;
  }
  virtual bool                 setDcsCode(      size_t   index, bits_t       value   ) {
    pMemory->channel[ index ].dcsCode = value;
    return true;
  }

  virtual bits_t               getDcsReverse(   size_t   index                       ) const {
    return pMemory->channel[ index ].overlay.dcsReverse;
  }
  virtual bool                 setDcsReverse(   size_t   index, bits_t       value   ) {
    pMemory->channel[ index ].overlay.dcsReverse = value;
    return true;
  }

  virtual bool                 getDisplay(      size_t   index                       ) const {
    return pMemory->channel[ index ].overlay.display != 0;
  }
  virtual bool                 setDisplay(      size_t   index, bool         value   ) {
    pMemory->channel[ index ].overlay.display = value ? 1 : 0;
    return true;
  }

  virtual bits_t               getDvSquelch(    size_t   index                       ) const {
    return pMemory->channel[ index ].dvSquelch;
  }
  virtual bool                 setDvSquelch(    size_t   index, bits_t       value   ) {
    pMemory->channel[ index ].dvSquelch = value;
    return true;
  }

  virtual bits_t               getDvCsqlCode(   size_t   index                       ) const {
    return pMemory->channel[ index ].dvCsqlCode;
  }
  virtual bool                 setDvCsqlCode(   size_t   index, bits_t       value   ) {
    pMemory->channel[ index ].dvCsqlCode = value;
    return true;
  }

  virtual Routing              getYourCall(     size_t   index                       ) const {
    size_t call = pMemory->channel[ index ].yourCall;
    if ( call == 0 ) {
      return Routing::cqcqcq;
    }
    if ( pMemory->channel[ index ].useRpt == 0 ) {
      return pMemory->urCall[ call - 1 ];
    } else {
      Routing rptCall = pMemory->rpCall[ call - 1 ];
      for ( int index = sizeof( rptCall.callsign ) - 2;  index > 0;  index-- ) {
        rptCall.callsign[ index ] = rptCall.callsign[ index - 1 ];
      }
      rptCall.callsign[ 0 ] = '/';
      return rptCall;
    }
  }
  virtual bool                 setYourCall(     size_t   index, char const * pValue ) {
    Routing yourCall = pValue;
    pMemory->channel[ index ].yourCall
        = yourCall.lookup( & Routing::cqcqcq, pMemory->urCall, COUNT_OF( pMemory->urCall ) - 1 );
    return true;
  }

  virtual Routing              getRpt1Call(     size_t   index                       ) const {
    size_t call = pMemory->channel[ index ].rpt1Call;
    return call != 0 ? pMemory->rpCall[ call - 1 ]
                     : Routing::notUse;
  }
  virtual bool                 setRpt1Call(     size_t   index, char const * pValue  ) {
    Routing rpt1Call = pValue;
    pMemory->channel[ index ].rpt1Call
        = rpt1Call.lookup( & Routing::notUse, pMemory->rpCall, COUNT_OF( pMemory->rpCall ) );
    return true;
  }

  virtual Routing              getRpt2Call(     size_t   index                       ) const {
    size_t call = pMemory->channel[ index ].rpt2Call;
    return call != 0 ? pMemory->rpCall[ call - 1 ]
                     : Routing::notUse;
  }
  virtual bool                 setRpt2Call(     size_t   index, char const * pValue  ) {
    Routing rpt2Call = pValue;
    pMemory->channel[ index ].rpt2Call
        = rpt2Call.lookup( & Routing::notUse, pMemory->rpCall, COUNT_OF( pMemory->rpCall ) );
    return true;
  }

  virtual bits_t               getBankGroup(    size_t   index                       ) const {
    return pMemory->suffix[ index ].bank;
  }
  virtual bool                 setBankGroup(    size_t   index, bits_t       value   )       {
    pMemory->suffix[ index ].bank = value < (int)Id800Memory::BANKS ? value : (int)Id800Memory::BANKS;
    return true;
  }

 public:
  Id800( char    const * pHeader,
         uint8_t const * pData,
         size_t          size )
      : Dstar( pHeader, pData, size ), pMemory( (Id800Memory *)Dstar::pData ) {
    assert( sizeof( Id800Channel ) == 22 );
  }

  virtual ~Id800( void ) {}
};

CsvField const Id800::csvFields[]
    = { { "CH No",           reinterpret_cast< getField_t >( & Id800::_getValid        ),
                             reinterpret_cast< setField_t >( & Id800::_setValid        ) },
        { "Frequency",       reinterpret_cast< getField_t >( & Id800::_getRxFreq       ),
                             reinterpret_cast< setField_t >( & Id800::_setRxFreq       ) },
        { "Dup",             reinterpret_cast< getField_t >( & Id800::_getSplit        ),
                             reinterpret_cast< setField_t >( & Id800::_setSplit        ) },
        { "Offset",          reinterpret_cast< getField_t >( & Id800::_getTxOffset     ),
                             reinterpret_cast< setField_t >( & Id800::_setTxOffset     ) },
        { "TS",              reinterpret_cast< getField_t >( & Id800::_getRxStep       ),
                             reinterpret_cast< setField_t >( & Id800::_setRxStep       ) },
        { "Power",           reinterpret_cast< getField_t >( & Id800::_getPowerLevel   ),
                             reinterpret_cast< setField_t >( & Id800::_setPowerLevel   ) },
        { "Mode",            reinterpret_cast< getField_t >( & Id800::_getModulation   ),
                             reinterpret_cast< setField_t >( & Id800::_setModulation   ) },
        { "Name",            reinterpret_cast< getField_t >( & Id800::_getName         ),
                             reinterpret_cast< setField_t >( & Id800::_setName         ) },
        { "SKIP",            reinterpret_cast< getField_t >( & Id800::_getSkipMode     ),
                             reinterpret_cast< setField_t >( & Id800::_setSkipMode     ) },
        { "TONE",            reinterpret_cast< getField_t >( & Id800::_getFmSquelch    ),
                             reinterpret_cast< setField_t >( & Id800::_setFmSquelch    ) },
        { "Repeater Tone",   reinterpret_cast< getField_t >( & Id800::_getCtcssEncode  ),
                             reinterpret_cast< setField_t >( & Id800::_setCtcssEncode  ) },
        { "TSQL Frequency",  reinterpret_cast< getField_t >( & Id800::_getCtcssDecode  ),
                             reinterpret_cast< setField_t >( & Id800::_setCtcssDecode  ) },
        { "DTCS Code",       reinterpret_cast< getField_t >( & Id800::_getDcsCode      ),
                             reinterpret_cast< setField_t >( & Id800::_setDcsCode      ) },
        { "DTCS Polarity",   reinterpret_cast< getField_t >( & Id800::_getDcsReverse  ),
                             reinterpret_cast< setField_t >( & Id800::_setDcsReverse  ) },
        { "DV SQL",          reinterpret_cast< getField_t >( & Id800::_getDvSquelch    ),
                             reinterpret_cast< setField_t >( & Id800::_setDvSquelch    ) },
        { "DV CSQL Code",    reinterpret_cast< getField_t >( & Id800::_getDvCsqlCode   ),
                             reinterpret_cast< setField_t >( & Id800::_setDvCsqlCode   ) },
        { "Your Call Sign",  reinterpret_cast< getField_t >( & Id800::_getYourCall     ),
                             reinterpret_cast< setField_t >( & Id800::_setYourCall     ) },
        { "RPT1 Call Sign",  reinterpret_cast< getField_t >( & Id800::_getRpt1Call     ),
                             reinterpret_cast< setField_t >( & Id800::_setRpt1Call     ) },
        { "RPT2 Call Sign",  reinterpret_cast< getField_t >( & Id800::_getRpt2Call     ),
                             reinterpret_cast< setField_t >( & Id800::_setRpt2Call     ) },
        { "Bank Group",      reinterpret_cast< getField_t >( & Id800::_getBankGroup    ),
                             reinterpret_cast< setField_t >( & Id800::_setBankChannel  ) },
        { 0, 0, 0 } };

Dstar * newId800( char    const * pHeader,
                  uint8_t const * pData,
                  size_t          size ) {
  return size == sizeof( Id800Memory )  &&  memcmp( pHeader, "27880200", 8 ) == 0
         ? new Id800( pHeader, pData, size ) : 0;
}
