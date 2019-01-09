//==================================================================================================
//  ThD74.cpp is the Kenwoork TH-D74 radio class file for Radio2csv.
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

#include "Dstar.hpp"

static char const * const booleans[]    = { "Off", "On" };
static char const * const splits[]      = {   " ",  "+",  "-",   "S",    "" };
static char const * const modulations[] = {  "FM", "DV", "AM", "LSB", "USB",  "CW", "NFM",  "DR",
                                            "WFM" };
static char const * const fmSquelches[] = { "Off",  "T", "CT", "DCS", "D_O", "T_D", "D_C", "T_C" };
static double       const tuneSteps[]   = {   5.0, 6.25, -2.0,  -3.0,  10.0,  12.5,  15.0,  20.0,
                                                   25.0, 30.0,  50.0, 100.0 };
static uint16_t     const fineSteps[]   = {    20,  100,  500,  1000  };

static struct {
  uint32_t const lower;
  uint32_t const upper;
} const bandLimits[] = { { 136000000, 174000000 },    //  0
                         { 216000000, 260000000 },    //  1
                         { 410000000, 470000000 },    //  2
                         {    100000,   1710000 },    //  3
                         {   1710000,  29700000 },    //  4
                         {  29700000,  76000000 },    //  5
                         {  76000000, 108000000 },    //  6
                         { 108000000, 136000000 },    //  7
                         {         0,         0 },
                         { 174000000, 216000000 },    //  9
                         { 260000000, 410000000 },    // 10
                         {         0,         0 },
                         { 470000000, 524000000 } };  // 12

struct ThD74Channel {  // little-endian target
  FrequencyLE< 32 >     rxFreq;                 // 0x00
  FrequencyLE< 32 >     txOffset;               // 0x04
  uint8_t LOW2HI_ORDER( txTuneStep    : 4,      // 0x08
                        rxTuneStep    : 4 );
  uint8_t LOW2HI_ORDER( fineStepValue : 2,      // 0x09
          LOW2HI_ORDER( fineStepOn    : 1,
                        modulation    : 5 ));
  uint8_t LOW2HI_ORDER( split         : 2,      // 0x0A
          LOW2HI_ORDER(               : 2,
                        fmSquelch     : 4 ));
  uint8_t               ctcssEncode;            // 0x0B
  uint8_t               ctcssDecode;            // 0x0C
  uint8_t               dcsCode;                // 0x0D
  uint8_t LOW2HI_ORDER( dvSquelch     : 2,      // 0x0E 
          LOW2HI_ORDER( dvMode        : 2,      //      DV=0, DR=1, else=3  
          LOW2HI_ORDER( crossSquelch  : 2,
                        drMode        : 2 )));  //      unknown, varies
  Routing               yourCall;               // 0x0F
  Routing               rpt1Call;               // 0x17
  Routing               rpt2Call;               // 0x1F
  uint8_t               dvCsqlCode;             // 0x27
};

struct ThD74ChannelSet {
  uint8_t               bandIndex;
  uint8_t               lockout;
  uint8_t               group;
  uint8_t               unknown;  // 0xFF
};

struct ThD74ChannelBlock {
  enum {
    BLOCK_SIZE = 6
  };

  ThD74Channel        channel[ BLOCK_SIZE ];
  uint8_t             fill[   16 ];
};

struct ThD74Memory {
  enum {
    CHANNELS   = 1000
  };

  uint8_t             fill1[       64 ];                                      // 0x000000
  char                comment2[    32 ];                                      // 0x000040
  uint8_t             fill2[    283*3 ];                                      // 0x000060
  MyCall              myCall[       6 ];                                      // 0x0003B1
  uint8_t             fill3[   2477*3 ];                                      // 0x0003F9
  ThD74ChannelSet     set[   CHANNELS ];                                      // 0x002100
  uint8_t             fill4[   131*32 ];                                      // 0x0030A0
  ThD74ChannelBlock   block[ CHANNELS / ThD74ChannelBlock::BLOCK_SIZE + 1 ];  // 0x004100
  uint8_t             fill5[   200*32 ];                                      // 0x00E800
  char                name[  CHANNELS ][ 16 ];                                // 0x010100
  uint8_t             fill6[  7500*32 ];                                      // 0x013F80
  char                comment1[    32 ];                                      // 0x04E900
  uint8_t             fill9[  5591*32 ];                                      // 0x04E920
};                                                                            // 0x07A400

class ThD74 : public Dstar {
  ThD74Memory * pMemory;

  ThD74(             void              );  // Intentionally not implemented
  ThD74(             ThD74 const & rhs );  // Intentionally not implemented
  ThD74 & operator=( ThD74 const & rhs );  // Intentionally not implemented

 protected:
  static CsvField     const csvFields[];

  virtual CsvField const * csvHeader( void ) const {
    return csvFields;
  }

  virtual bool   getIgnore(        size_t   index                       ) const { assert( false ); return 0; }
  virtual bool   setIgnore(        size_t   index, bool         value   )       { assert( false ); return 0; }
  virtual bool   getSkip(          size_t   index                       ) const { assert( false ); return 0; }
  virtual bool   setSkip(          size_t   index, bool         value   )       { assert( false ); return 0; }
  virtual bool   getSkipp(         size_t   index                       ) const { assert( false ); return 0; }
  virtual bool   setSkipp(         size_t   index, bool         value   )       { assert( false ); return 0; }

  virtual bits_t getDcsReverse(    size_t   index                       ) const { assert( false ); return 0; }
  virtual bool   setDcsReverse(    size_t   index, bits_t       value   )       { assert( false ); return 0; }
  virtual bits_t getBankGroup(     size_t   index                       ) const { assert( false ); return 0; }
  virtual bool   setBankGroup(     size_t   index, bits_t       value   )       { assert( false ); return 0; }
  virtual bits_t getBankChannel(   size_t   index                       ) const { assert( false ); return 0; }
  virtual bool   setBankChannel(   size_t   index, bits_t       value   )       { assert( false ); return 0; }

  virtual char const         * getComment(                      char       * pExport ) const {
    memcpy( pExport, pMemory->comment1, sizeof pMemory->comment1 );
    pExport[ sizeof pMemory->comment1 ] = 0xFF;
    *strchr( pExport, 0xFF ) = 0;
    return "Kenwood TH-D74";
  }
  virtual char const         * setComment(                      char const * pImport )       {
    if ( pImport != 0 ) {
      strpad( pMemory->comment1, sizeof pMemory->comment1, pImport, 0xFF );
      strpad( pMemory->comment2, sizeof pMemory->comment2, pImport, 0xFF );
    }
    return "Kenwood TH-D74";
  }
  virtual size_t               getCount(          void                               ) const {
    return ThD74Memory::CHANNELS;
  }

  virtual ThD74Channel       * getChannel(        size_t   index                       ) const {
    return & pMemory->block[ index / ThD74ChannelBlock::BLOCK_SIZE ].channel[ index % ThD74ChannelBlock::BLOCK_SIZE ];
  }

  virtual bool                 getValid(          size_t   index                       ) const {
    return pMemory->set[ index ].bandIndex != 0xFF;
  }
  virtual void                 setValid(          size_t   index, bool         value   ) {
    memset( getChannel( index ), value ? 0 : 0xFF, sizeof( ThD74Channel ) );
    memset( & pMemory->name[ index ], value ? ' ' : 0, sizeof pMemory->name[ 0 ] );
    pMemory->set[ index ].bandIndex = value ? COUNT_OF( bandLimits ) : 0xFF;
    pMemory->set[ index ].lockout   = 0;
    pMemory->set[ index ].group     = 0;
    pMemory->set[ index ].unknown   = 0xFF;
  }

  virtual uint32_t             getRxFreq(         size_t   index                       ) const {
    return getChannel( index )->rxFreq.hz();
  }
  virtual bool                 setRxFreq(         size_t   index, uint32_t     value   ) {
    for ( size_t bandIndex = 0;  bandIndex < COUNT_OF( bandLimits );  bandIndex++ ) {
      if ( bandLimits[ bandIndex ].lower <= value  &&  value < bandLimits[ bandIndex ].upper ) {
        pMemory->set[ index ].bandIndex = bandIndex;
        break;
      }
    }
    getChannel( index )->rxFreq.set( value );
    return true;
  }
  virtual uint32_t             getTxFreq(         size_t   index                       ) const {
    return getChannel( index )->rxFreq.hz();
  }
  virtual bool                 setTxFreq(         size_t   index, uint32_t     value   ) {
    return true;
  }

  virtual bits_t               getSplit(          size_t   index                       ) const {
    return getChannel( index )->split;
  }
  virtual bool                 setSplit(          size_t   index, bits_t       value   ) {
    getChannel( index )->split = value;
    return true;
  }
  virtual char const * const * getSplits(         size_t * count                       ) const {
    *count = COUNT_OF( ::splits );
    return ::splits;
  }

  virtual uint32_t             getTxOffset(       size_t   index                       ) const {
    return getChannel( index )->txOffset.hz();
  }
  virtual bool                 setTxOffset(       size_t   index, uint32_t     value   ) {
    getChannel( index )->txOffset.set( value );
    return true;
  }

  virtual bits_t               getRxStep(         size_t                      index  ) const {
    return getChannel( index )->rxTuneStep;
  }
  virtual bool                 setRxStep(         size_t   index, bits_t       value   ) {
    getChannel( index )->rxTuneStep = value;
    getChannel( index )->txTuneStep = value;
    return true;
  }
  virtual bits_t               getTxStep(         size_t                      index  ) const {
    return getChannel( index )->txTuneStep;
  }
  virtual bool                 setTxStep(         size_t   index, bits_t       value   ) {
//  getChannel( index )->txTuneStep = value;  -- Done in setRxStep()
    return true;
  }
  virtual double const       * getTuneSteps(      size_t * count                       ) const {
    *count = COUNT_OF( ::tuneSteps );
    return ::tuneSteps;
  }

  virtual bits_t               getModulation(     size_t   index                       ) const {
    return getChannel( index )->modulation >> 1;
  }
  virtual bool                 setModulation(     size_t   index, bits_t        value  ) {
    uint8_t dvMode = 3;
    getChannel( index )->modulation = value << 1;
    switch ( value ) {
      case 1:  // DV
        dvMode = 0;
        break;
      case 6:  // NFM
        getChannel( index )->modulation |= 1;
        break;
      case 7:  // DR
        dvMode = 1;
        break;
    }
    getChannel( index )->dvMode = dvMode;
    return true;
  }
  virtual char const * const * getModulations(    size_t * count                       ) const {
    *count = COUNT_OF( ::modulations );
    return ::modulations;
  }

  virtual char const * getName(                   size_t   index, size_t     * pSize   ) const {
    *pSize = sizeof pMemory->name[ 0 ];
    return pMemory->name[ index ];
  }
  virtual bool                 setName(           size_t   index, char const * pValue  ) {
    strpad( pMemory->name[ index ], sizeof pMemory->name[ 0 ], pValue, ' ');
    return true;
  }

  virtual bits_t               getFmSquelch(      size_t   index                       ) const {
    switch ( getChannel( index )->fmSquelch ) {
      case 0x0:
        return 0;
      case 0x8:
        return 1;
      case 0x4:
        return 2;
      case 0x2:
        return 3;
      case 0x1:
        break;
    }
    return 4 + getChannel( index )->crossSquelch;
  }
  virtual bool                 setFmSquelch(      size_t   index, bits_t       value   ) {
    if ( value < 4 ) {
      getChannel( index )->fmSquelch    = 0x10 >> value;
      getChannel( index )->crossSquelch = 0;
    } else {
      getChannel( index )->fmSquelch    = 1;
      getChannel( index )->crossSquelch = value - 4;
    }
    return true;
  }
  virtual char const * const * getFmSquelches(    size_t * count                       ) const {
    *count = COUNT_OF( ::fmSquelches );
    return ::fmSquelches;
  }

  virtual bits_t               getCtcssEncode(    size_t   index                       ) const {
    return getChannel( index )->ctcssEncode;
  }
  virtual bool                 setCtcssEncode(    size_t   index, bits_t       value   ) {
    getChannel( index )->ctcssEncode = value;
    return true;
  }

  virtual bits_t               getCtcssDecode(    size_t   index                       ) const {
    return getChannel( index )->ctcssDecode;
  }
  virtual bool                 setCtcssDecode(    size_t   index, bits_t       value   ) {
    getChannel( index )->ctcssDecode = value;
    return true;
  }

  virtual bits_t               getDcsCode(        size_t   index                       ) const {
    return getChannel( index )->dcsCode;
  }
  virtual bool                 setDcsCode(        size_t   index, bits_t       value   ) {
    getChannel( index )->dcsCode = value;
    return true;
  }

  virtual bool                _getDcsReverse(  size_t          index, char             * pExport ) const {
    strcpy( pExport, "----" );
    return true;
  }
  virtual bool                _setDcsReverse(  size_t          index, char const       * pImport )       {
    return strcmp( pImport, "----" ) == 0;
  }

  virtual bits_t               getSkipMode(       size_t   index                       ) const {
    return pMemory->set[ index ].lockout;
  }
  virtual bool                 setSkipMode(       size_t   index, bits_t       value   ) {
    pMemory->set[ index ].lockout = value;
    return true;
  }
  virtual bool                _getSkipMode(       size_t   index, char       * pExport ) const {
    size_t value = getSkipMode( index );
    if ( value >= sizeof booleans ) {
      sprintf( pExport, "%d", (int) value );
      return false;
    }
    strcpy( pExport, booleans[ value ] );
    return true;
  }
  virtual bool                _setSkipMode(       size_t   index, char const * pImport ) {
    int result  = search( pImport, booleans, COUNT_OF( booleans ) );
    setSkipMode( index, result >= 0 ? result : 0 );
    return result >= 0;
  }

  virtual bits_t               getFineStepOn(     size_t   index                       ) const {
    return getChannel( index )->fineStepOn;
  }
  virtual bool                 setFineStepOn(     size_t   index, bits_t       value   ) {
    getChannel( index )->fineStepOn = value;
    return true;
  }
  virtual bool                _getFineStepOn(     size_t   index, char       * pExport ) const {
    size_t value = getFineStepOn( index );
    if ( value >= sizeof booleans ) {
      sprintf( pExport, "%d", (int) value );
      return false;
    }
    strcpy( pExport, booleans[ value ] );
    return true;
  }
  virtual bool                _setFineStepOn(     size_t   index, char const * pImport ) {
    int result = search( pImport, booleans, COUNT_OF( booleans ) );
    setFineStepOn( index, result >= 0 ? result : 0 );
    return result >= 0;
  }

  virtual bits_t               getFineStepValue(  size_t   index                       ) const {
    return getChannel( index )->fineStepValue;
  }
  virtual bool                 setFineStepValue(  size_t   index, bits_t       value   ) {
    getChannel( index )->fineStepValue = value;
    return true;
  }
  virtual bool                _getFineStepValue(  size_t   index, char       * pExport ) const {
    size_t fineStep = getFineStepValue( index );
    if ( fineStep >= COUNT_OF( fineSteps ) ) {
      sprintf( pExport, "%d", (int) fineStep );
      return false;
    }
    sprintf( pExport, "%d", fineSteps[ fineStep ] );
    return true;
  }
  virtual bool                _setFineStepValue(  size_t   index, char const * pImport ) {
    char * pTemp;
    int result = search( strtoul( pImport, & pTemp, 10 ), fineSteps, COUNT_OF( fineSteps ) );
    if ( *pTemp != 0  ||  result < 0 ) {
      return false;
    }
    return setFineStepValue( index, result >= 0 ? result : 0 );
  }

  virtual bits_t               getGroup(          size_t   index                       ) const {
    return pMemory->set[ index ].group;
  }
  virtual bool                 setGroup(          size_t   index, bits_t       value   ) {
    pMemory->set[ index ].group = value;
    return true;
  }
  virtual bool                _getGroup(          size_t   index, char       * pExport ) const {
    size_t value = getGroup( index );
    sprintf( pExport, "%02d", (int)value );
    return value < 30;
  }
  virtual bool                _setGroup(          size_t   index, char const * pImport ) {
    char * pTemp;
    int result = strtoul( pImport, & pTemp, 10 );
    if ( *pTemp != 0  ||  result < 0  ||  result > 29 ) {
      return false;
    }
    return setGroup( index, result );
  }

  virtual bits_t               getDvSquelch(      size_t   index                       ) const {
    return getChannel( index )->dvSquelch;
  }
  virtual bool                 setDvSquelch(      size_t   index, bits_t       value   ) {
    getChannel( index )->dvSquelch = value;
    return true;
  }

  virtual bits_t               getDvCsqlCode(     size_t   index                       ) const {
    return getChannel( index )->dvCsqlCode;
  }
  virtual bool                 setDvCsqlCode(     size_t   index, bits_t       value   ) {
    getChannel( index )->dvCsqlCode = value;
    return true;
  }

  virtual Routing              getYourCall(       size_t   index                       ) const {
    return getChannel( index )->yourCall;
  }
  virtual bool                 setYourCall(       size_t   index, char const * pValue ) {
    getChannel( index )->yourCall = pValue;
    return true;
  }

  virtual Routing              getRpt1Call(       size_t   index                       ) const {
    Routing routing = getChannel( index )->rpt1Call;
    return routing == Routing::direct ? Routing::notUse : routing;
  }
  virtual bool                 setRpt1Call(       size_t   index, char const * pValue  ) {
    Routing routing = pValue;
    getChannel( index )->rpt1Call
        = routing == Routing::notUse ? Routing::direct : routing;
    return true;
  }

  virtual Routing              getRpt2Call(       size_t   index                       ) const {
    Routing routing = getChannel( index )->rpt2Call;
    return routing == Routing::direct ? Routing::notUse : routing;
  }
  virtual bool                 setRpt2Call(       size_t   index, char const * pValue  ) {
    Routing routing = pValue;
    getChannel( index )->rpt2Call
        = routing == Routing::notUse ? Routing::direct : routing;
    return true;
  }

 public:
  ThD74( char    const * pHeader,
         uint8_t const * pData,
         size_t          size )
      : Dstar( pHeader, pData, size ), pMemory( (ThD74Memory *)Dstar::pData ) {
    assert( sizeof( ThD74Channel ) == 40 );
  }

  virtual ~ThD74( void ) {}
};

CsvField const ThD74::csvFields[]
    = { { "Channel Number",      reinterpret_cast< getField_t >( & ThD74::_getValid          ),
                                 reinterpret_cast< setField_t >( & ThD74::_setValid          ) },
        { "Receive Frequency",   reinterpret_cast< getField_t >( & ThD74::_getRxFreq         ),
                                 reinterpret_cast< setField_t >( & ThD74::_setRxFreq         ) },
        { "Receive Step",        reinterpret_cast< getField_t >( & ThD74::_getRxStep         ),
                                 reinterpret_cast< setField_t >( & ThD74::_setRxStep         ) },
        { "Offset Frequency",    reinterpret_cast< getField_t >( & ThD74::_getTxOffset       ),
                                 reinterpret_cast< setField_t >( & ThD74::_setTxOffset       ) },
        { "T/CT/DCS",            reinterpret_cast< getField_t >( & ThD74::_getFmSquelch      ),
                                 reinterpret_cast< setField_t >( & ThD74::_setFmSquelch      ) },
        { "Tone",                reinterpret_cast< getField_t >( & ThD74::_getCtcssEncode    ),
                                 reinterpret_cast< setField_t >( & ThD74::_setCtcssEncode    ) },
        { "CTCSS",               reinterpret_cast< getField_t >( & ThD74::_getCtcssDecode    ),
                                 reinterpret_cast< setField_t >( & ThD74::_setCtcssDecode    ) },
        { "DCS",                 reinterpret_cast< getField_t >( & ThD74::_getDcsCode        ),
                                 reinterpret_cast< setField_t >( & ThD74::_setDcsCode        ) },
        { "Shift/Split",         reinterpret_cast< getField_t >( & ThD74::_getSplit          ),
                                 reinterpret_cast< setField_t >( & ThD74::_setSplit          ) },
        { "Reverse",             reinterpret_cast< getField_t >( & ThD74::_getDcsReverse     ),
                                 reinterpret_cast< setField_t >( & ThD74::_setDcsReverse     ) },
        { "Lockout",             reinterpret_cast< getField_t >( & ThD74::_getSkipMode       ),
                                 reinterpret_cast< setField_t >( & ThD74::_setSkipMode       ) },
        { "Operating Mode",      reinterpret_cast< getField_t >( & ThD74::_getModulation     ),
                                 reinterpret_cast< setField_t >( & ThD74::_setModulation     ) },
        { "Transmit Frequency",  reinterpret_cast< getField_t >( & ThD74::_getTxFreq         ),
                                 reinterpret_cast< setField_t >( & ThD74::_setTxFreq         ) },
        { "Transmit Step",       reinterpret_cast< getField_t >( & ThD74::_getTxStep         ),
                                 reinterpret_cast< setField_t >( & ThD74::_setTxStep         ) },
        { "Name",                reinterpret_cast< getField_t >( & ThD74::_getName           ),
                                 reinterpret_cast< setField_t >( & ThD74::_setName           ) },
        { "Fine Step Enable",    reinterpret_cast< getField_t >( & ThD74::_getFineStepOn     ),
                                 reinterpret_cast< setField_t >( & ThD74::_setFineStepOn     ) },
        { "Fine Step",           reinterpret_cast< getField_t >( & ThD74::_getFineStepValue  ),
                                 reinterpret_cast< setField_t >( & ThD74::_setFineStepValue  ) },
        { "Group",               reinterpret_cast< getField_t >( & ThD74::_getGroup          ),
                                 reinterpret_cast< setField_t >( & ThD74::_setGroup          ) },
        { "Digital Squelch",     reinterpret_cast< getField_t >( & ThD74::_getDvSquelch      ),
                                 reinterpret_cast< setField_t >( & ThD74::_setDvSquelch      ) },
        { "Digital Code",        reinterpret_cast< getField_t >( & ThD74::_getDvCsqlCode     ),
                                 reinterpret_cast< setField_t >( & ThD74::_setDvCsqlCode     ) },
        { "Your Callsign",       reinterpret_cast< getField_t >( & ThD74::_getYourCall       ),
                                 reinterpret_cast< setField_t >( & ThD74::_setYourCall       ) },
        { "Rpt-1 Callsign",      reinterpret_cast< getField_t >( & ThD74::_getRpt1Call       ),
                                 reinterpret_cast< setField_t >( & ThD74::_setRpt1Call       ) },
        { "Rpt-2 Callsign",      reinterpret_cast< getField_t >( & ThD74::_getRpt2Call       ),
                                 reinterpret_cast< setField_t >( & ThD74::_setRpt2Call       ) },
        { 0, 0, 0 } };

Dstar * newThD74( char    const * pHeader,
                       uint8_t const * pData,
                       size_t          size ) {
  return size == sizeof( ThD74Memory )
         ? new ThD74( pHeader, pData, size ) : 0;
}
