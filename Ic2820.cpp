//==================================================================================================
//  Ic2820.cpp is the Icom IC-2820H radio class file for Radio2csv.
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

static char const * const modulations[] = {  "FM", "FM-N",      "AM", "AM-N",     "DV",     "?5?",  "?6?",    "?7?"  };
static char const * const fmSquelches[] = { "OFF", "TONE", "TSQL(*)", "TSQL", "TSQL-R", "DTCS(*)", "DTCS", "DTCS-R"  };
static double       const tuneSteps[]   = {   5.0,   6.25,      10.0,   12.5,     15.0,      20.0,   25.0,     30.0,
                                             50.0,   -9.0,     -10.0,  -11.0,    -12.0,     -13.0,  -14.0,    -15.0  };

struct Ic2820Channel {   // big-endian target
  FrequencyBE< 32 >     rxFreq;
  FrequencyBE< 32 >     txOffset;
  Routing               yourCall;
  Routing               rpt1Call;
  Routing               rpt2Call;
  uint8_t               unknown1;           // sometimes 0x72, sometimes 0xFF
  uint8_t HI2LOW_ORDER(                : 1,
          HI2LOW_ORDER( direction      : 2,
          HI2LOW_ORDER( fmSquelch      : 3,
                                       : 2 )));
  uint8_t HI2LOW_ORDER( ctcssDecode    : 6,
                        ctcssEncodeHi  : 2 );
  uint8_t HI2LOW_ORDER( ctcssEncodeLo  : 4,
                        tuneStep       : 4 );
  uint8_t HI2LOW_ORDER( dcsCode        : 7,
                        modulationHi   : 1 );
  uint8_t HI2LOW_ORDER( modulationLo   : 2,
          HI2LOW_ORDER(                : 2,
                        unknown2       : 4 )); // 0x9 if rxFreq % 5000 != 0
  uint8_t HI2LOW_ORDER( dvCsqlCode     : 7,
                        dvSquelchHi    : 1 );
  uint8_t HI2LOW_ORDER( dvSquelchLo    : 1,
          HI2LOW_ORDER(                : 1,
          HI2LOW_ORDER( dcsReverse     : 2,
                                       : 4 )));
  char                  name[ 8 ];
};

struct Ic2820Memory {
  enum {
    CHANNELS = 500,
    BANKS    =  26,
    EDGES    =  10
  };

  Ic2820Channel channel[ CHANNELS ];  // 0x0000
  struct {
    Ic2820Channel  lower,
                   upper;
  }              scanEdge[ EDGES ];    // 0x5DC0
  Ic2820Channel  call[      2 ];       // 0x6180
  uint8_t        ignoreChannel[ (CHANNELS + EDGES * 2 - 1) / CHAR_BIT + 1 ];  // 0x61E0
  uint8_t        fill1[     1 ];                                              // 0x6221
  uint8_t        skipChannel[   (CHANNELS + EDGES * 2 - 1) / CHAR_BIT + 1 ];  // 0x6222
  uint8_t        skippChannel[  (CHANNELS + EDGES * 2 - 1) / CHAR_BIT + 1 ];  // 0x6263
  BankMap        bankMap[        CHANNELS + EDGES * 2 ];                      // 0x62A4
  uint8_t        fill2[   684 ];       // 0x66B4
  char           comment[  16 ];       // 0x6960
  MyCall         myCall[    6 ];       // 0x6970
  Routing        urCall[   60 ];       // 0x69B8
  Routing        rpCall[   60 ];       // 0x6B98
  uint8_t        fill9[ 16200 ];       // 0x6D78
};                                     // 0xACC0

class Ic2820 : public Dstar {
  Ic2820Memory * pMemory;

  Ic2820(             void               );  // Intentionally not implemented
  Ic2820(             Ic2820 const & rhs );  // Intentionally not implemented
  Ic2820 & operator=( Ic2820 const & rhs );  // Intentionally not implemented
 protected:
  virtual bool                 getIgnore(       size_t   index                       ) const {
    return getBit( pMemory->ignoreChannel, index );
  }
  virtual bool                 setIgnore(       size_t   index, bool         value   )       {
    return setBit( pMemory->ignoreChannel, index, value );
  }
  virtual bool                 getSkip(         size_t   index                       ) const {
    return getBit( pMemory->skipChannel,   index );
  }
  virtual bool                 setSkip(         size_t   index, bool         value   )       {
    return setBit( pMemory->skipChannel,   index, value );
  }
  virtual bool                 getSkipp(        size_t   index                       ) const {
    return getBit( pMemory->skippChannel,  index );
  }
  virtual bool                 setSkipp(        size_t   index, bool         value   )       {
    return setBit( pMemory->skippChannel,  index, value );
  }

  virtual char const         * getComment(                      char       * pExport ) const {
    memcpy( pExport, pMemory->comment, sizeof pMemory->comment );
    pExport[ sizeof pMemory->comment ] = 0;
    return "Icom IC-2820H";
  }
  virtual char const         * setComment(                      char const * pImport )       {
    if ( pImport != 0 ) {
      strpad( pMemory->comment, sizeof pMemory->comment, pImport, ' ' );
    }
    return "Icom IC-2820H";
  }
  virtual size_t               getCount(        void                                 ) const {
    return Ic2820Memory::CHANNELS;
  }

  virtual bool                 getValid(        size_t   index                       ) const {
    return !getIgnore( index );
  }
  virtual void                 setValid(        size_t   index, bool         value   ) {
    memset( & pMemory->channel[ index ], value ? 0 : -1, sizeof pMemory->channel[ 0 ] );
    setRxFreq(   index, 5000 );
    setTxOffset( index, 5000 );
    setName(     index, ""     );
    setSkipMode( index, 0 );
    pMemory->channel[ index ].unknown1 = 0xFF;
    pMemory->channel[ index ].direction = 0;
    setIgnore( index, !value );
  }

  virtual uint32_t             getRxFreq(       size_t   index                       ) const {
    return pMemory->channel[ index ].rxFreq.hz();
  }
  virtual bool                 setRxFreq(       size_t   index, uint32_t     value   ) {
    pMemory->channel[ index ].rxFreq.set( value );
    return true;
  }

  virtual bits_t               getSplit(        size_t   index                       ) const {
    return pMemory->channel[ index ].direction;
  }
  virtual bool                 setSplit(        size_t   index, bits_t       value   ) {
    pMemory->channel[ index ].direction = value;
    return true;
  }

  virtual uint32_t             getTxOffset(     size_t   index                       ) const {
    return pMemory->channel[ index ].txOffset.hz();
  }
  virtual bool                 setTxOffset(     size_t   index, uint32_t     value   ) {
    pMemory->channel[ index ].txOffset.set( value );
    return true;
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

  virtual bits_t               getModulation(   size_t   index                       ) const {
    return pMemory->channel[ index ].modulationHi << 2 | pMemory->channel[ index ].modulationLo;
  }
  virtual bool                 setModulation(   size_t   index, bits_t       value   ) {
    pMemory->channel[ index ].modulationLo = value;
    pMemory->channel[ index ].modulationHi = value >> 2;
    return true;
  }
  virtual char const * const * getModulations(  size_t * count                       ) const {
    *count = COUNT_OF( ::modulations );
    return ::modulations;
  }

  virtual char const         * getName(         size_t   index, size_t     * pSize   ) const {
    *pSize = sizeof pMemory->channel[ 0 ].name;
    return pMemory->channel[ index ].name;
  }
  virtual bool                 setName(         size_t   index, char const * pValue  ) {
    strpad( pMemory->channel[ index ].name, sizeof pMemory->channel[ 0 ].name, pValue, ' ');
    return true;
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
    return pMemory->channel[ index ].ctcssEncodeHi << 4 | pMemory->channel[ index ].ctcssEncodeLo;
  }
  virtual bool                 setCtcssEncode(  size_t   index, bits_t       value   ) {
    pMemory->channel[ index ].ctcssEncodeLo = value;
    pMemory->channel[ index ].ctcssEncodeHi = value >> 4;
    return true;
  }

  virtual bits_t               getCtcssDecode(  size_t   index  ) const {
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
    return pMemory->channel[ index ].dcsReverse;
  }
  virtual bool                 setDcsReverse(   size_t   index, bits_t       value   ) {
    pMemory->channel[ index ].dcsReverse = value;
    return true;
  }

  virtual bits_t               getDvSquelch(    size_t   index                       ) const {
    return pMemory->channel[ index ].dvSquelchHi << 1 | pMemory->channel[ index ].dvSquelchLo;
  }
  virtual bool                 setDvSquelch(    size_t   index, bits_t       value   ) {
    pMemory->channel[ index ].dvSquelchLo = value;
    pMemory->channel[ index ].dvSquelchHi = value >> 1;
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
    return pMemory->channel[ index ].yourCall;
  }
  virtual bool                 setYourCall(     size_t   index, char const * pValue  ) {
    pMemory->channel[ index ].yourCall = pValue;
    pMemory->channel[ index ].yourCall.lookup( & Routing::cqcqcq,
                                               pMemory->urCall, COUNT_OF( pMemory->urCall ) );
    return true;
  }

  virtual Routing              getRpt1Call(     size_t   index                       ) const {
    return pMemory->channel[ index ].rpt1Call;
  }
  virtual bool                 setRpt1Call(     size_t   index, char const * pValue  ) {
    pMemory->channel[ index ].rpt1Call = pValue;
    pMemory->channel[ index ].rpt1Call.lookup( & Routing::notUse,
                                               pMemory->rpCall, COUNT_OF( pMemory->rpCall ) );
    return true;
  }

  virtual Routing              getRpt2Call(     size_t   index                       ) const {
    return pMemory->channel[ index ].rpt2Call;
  }
  virtual bool                 setRpt2Call(     size_t   index, char const * pValue  ) {
    pMemory->channel[ index ].rpt2Call = pValue;
    pMemory->channel[ index ].rpt2Call.lookup( & Routing::notUse,
                                               pMemory->rpCall, COUNT_OF( pMemory->rpCall ) );
    return true;
  }

  virtual bits_t             getBankGroup(    size_t   index                       ) const {
    return pMemory->bankMap[ index ].group;
  }
  virtual bool               setBankGroup(    size_t   index, bits_t       value   )       {
    pMemory->bankMap[ index ].group = value < (int)Ic2820Memory::BANKS ? value : (int)Ic2820Memory::BANKS;
    return true;
  }

  virtual bits_t             getBankChannel(  size_t   index                       ) const {
    return pMemory->bankMap[ index ].index;
  }
  virtual bool               setBankChannel(  size_t   index, bits_t       value   )       {
    pMemory->bankMap[ index ].index = value;
    return true;
  }

 public:
  Ic2820( char    const * pHeader,
          uint8_t const * pData,
          size_t          size )
      : Dstar( pHeader, pData, size ), pMemory( (Ic2820Memory *)Dstar::pData ) {
      assert( sizeof( Ic2820Channel ) == 48 );
  }
  virtual ~Ic2820( void ) {}
};

Dstar * newIc2820( char    const * pHeader,
                   uint8_t const * pData,
                   size_t          size ) {
  return size == sizeof( Ic2820Memory )  &&  memcmp( pHeader, "29700001", 8 ) == 0
         ? new Ic2820( pHeader, pData, size ) : 0;
}
