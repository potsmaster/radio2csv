//==================================================================================================
//  Ic9Xd.cpp is the Icom IC-91/92AD radio class definition file for Radio2csv.
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

struct Ic9XdChannel {     // big-endian target
  FrequencyBE< 24 >     rxFreq;
  FrequencyBE< 16 >     txOffset;
  uint8_t HI2LOW_ORDER( ctcssEncode   : 6,
                        ctcssDecodeHi : 2 );
  uint8_t HI2LOW_ORDER( ctcssDecodeLo : 4,
          HI2LOW_ORDER(               : 1,
                        modulation    : 3 ));
  uint8_t HI2LOW_ORDER(               : 1,
                        dcsCode       : 7 );
  uint8_t HI2LOW_ORDER( tuneStep      : 4,
          HI2LOW_ORDER( dvSquelch     : 2,
                                      : 2 ));
  uint8_t               unknown1;          // 0xE4
  uint8_t HI2LOW_ORDER(               : 3,
          HI2LOW_ORDER( rxResolution  : 1,
          HI2LOW_ORDER(               : 3,
                        txResolution  : 1 )));
  uint8_t HI2LOW_ORDER( fmSquelch     : 4,
          HI2LOW_ORDER( direction     : 2,
                        dcsReverse    : 2 ));
  char                  name[ 8 ];
};

struct Ic9XdChannelA : Ic9XdChannel {
};

struct Ic9XdChannelB : Ic9XdChannel {
  uint8_t dvCsqlCode;
  Routing yourCall;
  Routing rpt1Call;
  Routing rpt2Call;
};

struct Ic9XdMemory {
  enum {
    A_CHANNELS = 800,
    B_CHANNELS = 400,
    BANKS      =  26,
    EDGES      =  25
  };

  Ic9XdChannelA channelA[ A_CHANNELS ];  // 0x0000
  struct {
    Ic9XdChannelA lower,
                  upper;
  }             scanEdgeA[ EDGES ];       // 0x3E80
  Ic9XdChannelB channelB[ B_CHANNELS ];  // 0x4268
  struct {
    Ic9XdChannelB lower,
                  upper;
  }             scanEdgeB[ EDGES ];       // 0x88B8
  uint8_t       fill1[   620 ];           // 0x9182
  uint8_t       ignoreChannelA[ (A_CHANNELS + EDGES * 2 - 1) / CHAR_BIT + 1 ];  // 0x93EE
  uint8_t       skipChannelA[   (A_CHANNELS + EDGES * 2 - 1) / CHAR_BIT + 1 ];  // 0x9459
  uint8_t       skippChannelA[  (A_CHANNELS + EDGES * 2 - 1) / CHAR_BIT + 1 ];  // 0x94C4
  uint8_t       ignoreChannelB[ (B_CHANNELS + EDGES * 2 - 1) / CHAR_BIT + 1 ];  // 0x952F
  uint8_t       skipChannelB[   (B_CHANNELS + EDGES * 2 - 1) / CHAR_BIT + 1 ];  // 0x9568
  uint8_t       skippChannelB[  (B_CHANNELS + EDGES * 2 - 1) / CHAR_BIT + 1 ];  // 0x95A1
  uint8_t       fill2[    18 ];                                                 // 0x95DA
  BankMap       bankMapA[        A_CHANNELS + EDGES * 2 ];                      // 0x95EC
  BankMap       bankMapB[        B_CHANNELS + EDGES * 2 ];                      // 0x9C90
  uint8_t       fill3[ 61*16 ];           // 0xA014
  MyCall        myCall[    6 ];           // 0xA3E4
  Routing       urCall[   60 ];           // 0xA42C
  Routing       rpCall[   60 ];           // 0xA60C
};                                        // 0xA7EC

class Ic9Xd : public Dstar {
  Ic9XdMemory * pMemory;

  virtual Ic9XdChannel       * getChannel(      size_t   index  ) const {
    return index < Ic9XdMemory::A_CHANNELS ? (Ic9XdChannel *) & pMemory->channelA[ index ]
                                           : (Ic9XdChannel *) & pMemory->channelB[ index - Ic9XdMemory::A_CHANNELS ];
  }
  virtual BankMap            * getBankMap(      size_t   index  ) const {
    return index < Ic9XdMemory::A_CHANNELS ? & pMemory->bankMapA[ index ]
                                           : & pMemory->bankMapB[ index - Ic9XdMemory::A_CHANNELS ];
  }

  Ic9Xd(             void              );  // Intentionally not implemented
  Ic9Xd(             Ic9Xd const & rhs );  // Intentionally not implemented
  Ic9Xd & operator=( Ic9Xd const & rhs );  // Intentionally not implemented

 protected:
  static char const * const modulations[ 8 ];
  virtual bool                 getIgnore(       size_t   index                       ) const {
    return index < Ic9XdMemory::A_CHANNELS ? getBit( pMemory->ignoreChannelA, index              )
                                           : getBit( pMemory->ignoreChannelB, index - Ic9XdMemory::A_CHANNELS );
  }
  virtual bool                 setIgnore(       size_t   index, bool         value   )       {
    return index < Ic9XdMemory::A_CHANNELS ? setBit( pMemory->ignoreChannelA, index,                           value )
                                           : setBit( pMemory->ignoreChannelB, index - Ic9XdMemory::A_CHANNELS, value );
  }
  virtual bool                 getSkip(         size_t   index                       ) const {
    return index < Ic9XdMemory::A_CHANNELS ? getBit( pMemory->skipChannelA,   index              )
                                           : getBit( pMemory->skipChannelB,   index - Ic9XdMemory::A_CHANNELS );
  }
  virtual bool                 setSkip(         size_t   index, bool         value   )       {
    return index < Ic9XdMemory::A_CHANNELS ? setBit( pMemory->skipChannelA,   index,                           value )
                                           : setBit( pMemory->skipChannelB,   index - Ic9XdMemory::A_CHANNELS, value );
  }
  virtual bool                 getSkipp(        size_t   index                       ) const {
    return index < Ic9XdMemory::A_CHANNELS ? getBit( pMemory->skippChannelA,  index              )
                                           : getBit( pMemory->skippChannelB,  index - Ic9XdMemory::A_CHANNELS );
  }
  virtual bool                 setSkipp(        size_t   index, bool         value   )       {
    return index < Ic9XdMemory::A_CHANNELS ? setBit( pMemory->skippChannelA,  index,                           value )
                                           : setBit( pMemory->skippChannelB,  index - Ic9XdMemory::A_CHANNELS, value );
  }

  virtual size_t               getCount(        void                                 ) const {
    return Ic9XdMemory::A_CHANNELS + Ic9XdMemory::B_CHANNELS;
  }

  virtual bool                 getValid(        size_t   index                       ) const {
    return !getIgnore( index );
  }
  virtual void                 setValid(        size_t   index, bool         value   ) {
    memset( getChannel( index ), value ? 0 : -1, index < Ic9XdMemory::A_CHANNELS ? sizeof pMemory->channelA[ 0 ]
                                                                                 : sizeof pMemory->channelB[ 0 ] );
    memset( getBankMap( index ), value ? 0 : -1,                                   sizeof( BankMap ) );
    setName(     index, "" );
    setSkipMode( index, 0 );
    getChannel( index )->unknown1 = 0xE4;
    setIgnore( index, !value );
  }

  virtual uint32_t             getRxFreq(       size_t   index                       ) const {
    return getChannel( index )->rxFreq.hz( divisorsX3[ getChannel( index )->rxResolution ] ) / 3;
  }
  virtual bool                 setRxFreq(       size_t   index, uint32_t     value   ) {
    int    result     = search( value * 3, divisorsX3, COUNT_OF( divisorsX3 ) );
    bits_t divisor = result >= 0 ? result : 0;
    getChannel( index )->rxResolution = divisor;
    getChannel( index )->rxFreq.set( value * 3, divisorsX3[ divisor ] );
    return result >= 0;
  }

  virtual bits_t               getSplit(        size_t   index                       ) const {
    return getChannel( index )->direction;
  }
  virtual bool                 setSplit(        size_t   index, bits_t       value   ) {
    getChannel( index )->direction = value;
    return true;
  }

  virtual uint32_t             getTxOffset(     size_t   index                       ) const {
    return getChannel( index )->txOffset.hz( divisorsX3[ getChannel( index )->txResolution ] ) / 3;
  }
  virtual bool                 setTxOffset(     size_t   index, uint32_t     value   ) {
    int    result     = search( value * 3, divisorsX3, COUNT_OF( divisorsX3 ) );
    bits_t divisor = result >= 0 ? result : 0;
    getChannel( index )->txResolution = divisor;
    getChannel( index )->txOffset.set( value * 3, divisorsX3[ divisor ] );
    return result >= 0;
  }

  virtual double const       * getTuneSteps(    size_t * count                       ) const {
    *count = COUNT_OF( tuneSteps );
    return tuneSteps;
  }
  virtual bits_t               getRxStep(       size_t   index                       ) const {
    return getChannel( index )->tuneStep ;
  }
  virtual bool                 setRxStep(       size_t   index, bits_t       value   ) {
    getChannel( index )->tuneStep = value;
    return true;
  }

  virtual char const * const * getModulations(  size_t * count                       ) const {
    *count = COUNT_OF( modulations );
    return modulations;
  }
  virtual bits_t               getModulation(   size_t   index                       ) const {
    return getChannel( index )->modulation;
  }
  virtual bool                 setModulation(   size_t   index, bits_t       value   ) {
    getChannel( index )->modulation = value;
    return true;
  }

  virtual char const         * getName(         size_t   index, size_t     * pSize   ) const {
    *pSize = sizeof pMemory->channelA[ 0 ].name;
    return getChannel( index )->name;
  }
  virtual bool                 setName(         size_t   index, char const * pValue  ) {
    strpad( getChannel( index )->name, sizeof pMemory->channelA[ 0 ].name, pValue, ' ' );
    return true;
  }

  virtual char const * const * getFmSquelches(  size_t * count                       ) const {
    *count = COUNT_OF( fmSquelches );
    return fmSquelches;
  }
  virtual bits_t               getFmSquelch(    size_t   index                       ) const {
    return getChannel( index )->fmSquelch;
  }
  virtual bool                 setFmSquelch(    size_t   index, bits_t       value   ) {
    getChannel( index )->fmSquelch = value;
    return true;
  }

  virtual bits_t               getCtcssEncode(  size_t   index                       ) const {
    return getChannel( index )->ctcssEncode;
  }
  virtual bool                 setCtcssEncode(  size_t   index, bits_t       value   ) {
    getChannel( index )->ctcssEncode = value;
    return true;
  }

  virtual bits_t               getCtcssDecode(  size_t   index                       ) const {
    return getChannel( index )->ctcssDecodeHi << 4 | getChannel( index )->ctcssDecodeLo;
  }
  virtual bool                 setCtcssDecode(  size_t   index, bits_t       value   ) {
    getChannel( index )->ctcssDecodeLo = value;
    getChannel( index )->ctcssDecodeHi = value >> 4;
    return true;
  }

  virtual bits_t               getDcsCode(      size_t   index                       ) const {
    return getChannel( index )->dcsCode;
  }
  virtual bool                 setDcsCode(      size_t   index, bits_t       value   ) {
    getChannel( index )->dcsCode = value;
    return true;
  }

  virtual bits_t               getDcsReverse(   size_t   index                       ) const {
    return getChannel( index )->dcsReverse;
  }
  virtual bool                 setDcsReverse(   size_t   index, bits_t       value   ) {
    getChannel( index )->dcsReverse = value;
    return true;
  }

  virtual bits_t               getDvSquelch(    size_t   index                       ) const {
    return index >= Ic9XdMemory::A_CHANNELS ? pMemory->channelB[ index - Ic9XdMemory::A_CHANNELS ].dvSquelch : 0;
  }
  virtual bool                 setDvSquelch(    size_t   index, bits_t       value   ) {
    if ( index >= Ic9XdMemory::A_CHANNELS ) {
      pMemory->channelB[ index - Ic9XdMemory::A_CHANNELS ].dvSquelch = value;
    }
    return true;
  }

  virtual bits_t               getDvCsqlCode(   size_t   index                       ) const {
    return index >= Ic9XdMemory::A_CHANNELS ? pMemory->channelB[ index - Ic9XdMemory::A_CHANNELS ].dvCsqlCode : 0;
  }
  virtual bool                 setDvCsqlCode(   size_t   index, bits_t       value   ) {
    if ( index >= Ic9XdMemory::A_CHANNELS ) {
      pMemory->channelB[ index - Ic9XdMemory::A_CHANNELS ].dvCsqlCode = value;
    }
    return true;
  }

  virtual Routing              getYourCall(     size_t   index                       ) const {
    return index >= Ic9XdMemory::A_CHANNELS ? pMemory->channelB[ index - Ic9XdMemory::A_CHANNELS ].yourCall
                                            : Routing::cqcqcq;
  }
  virtual bool                 setYourCall(     size_t   index, char const * pValue  ) {
    if ( index >= Ic9XdMemory::A_CHANNELS ) {
      pMemory->channelB[ index - Ic9XdMemory::A_CHANNELS ].yourCall = pValue;
      pMemory->channelB[ index - Ic9XdMemory::A_CHANNELS ].yourCall
          .lookup( & Routing::cqcqcq, pMemory->urCall, COUNT_OF( pMemory->urCall ) );
    }
    return true;
  }

  virtual Routing              getRpt1Call(     size_t   index                       ) const {
    return index >= Ic9XdMemory::A_CHANNELS ? pMemory->channelB[ index - Ic9XdMemory::A_CHANNELS ].rpt1Call
                                            : Routing::notUse;
  }
  virtual bool                 setRpt1Call(     size_t   index, char const * pValue  ) {
    if ( index >= Ic9XdMemory::A_CHANNELS ) {
      pMemory->channelB[ index - Ic9XdMemory::A_CHANNELS ].rpt1Call = pValue;
      pMemory->channelB[ index - Ic9XdMemory::A_CHANNELS ].rpt1Call
          .lookup( & Routing::cqcqcq, pMemory->rpCall, COUNT_OF( pMemory->rpCall ) );
    }
    return true;
  }

  virtual Routing              getRpt2Call(     size_t   index                       ) const {
    return index >= Ic9XdMemory::A_CHANNELS ? pMemory->channelB[ index - Ic9XdMemory::A_CHANNELS ].rpt2Call
                                            : Routing::notUse;
  }
  virtual bool                 setRpt2Call(     size_t   index, char const * pValue  ) {
    if ( index >= Ic9XdMemory::A_CHANNELS ) {
      pMemory->channelB[ index - Ic9XdMemory::A_CHANNELS ].rpt2Call = pValue;
      pMemory->channelB[ index - Ic9XdMemory::A_CHANNELS ].rpt2Call
          .lookup( & Routing::cqcqcq, pMemory->rpCall, COUNT_OF( pMemory->rpCall ) );
    }
    return true;
  }

  virtual bits_t             getBankGroup(    size_t   index                       ) const {
    return getBankMap( index )->group;
  }
  virtual bool               setBankGroup(    size_t   index, bits_t       value   )       {
    getBankMap( index )->group = value < (int)Ic9XdMemory::BANKS ? value : (int)Ic9XdMemory::BANKS;
    return true;
  }

  virtual bits_t             getBankChannel(  size_t   index                       ) const {
    return getBankMap( index )->index;
  }
  virtual bool               setBankChannel(  size_t   index, bits_t       value   )       {
    getBankMap( index )->index = value;
    return true;
  }

 public:
  Ic9Xd( char    const * pHeader,
         uint8_t const * pData,
         size_t          size )
      : Dstar( pHeader, pData, size ), pMemory( (Ic9XdMemory *)Dstar::pData ) {
    assert( sizeof( Ic9XdChannelA ) == 20 );
    assert( sizeof( Ic9XdChannelB ) == 45 );
    assert( sizeof( Ic9XdMemory   ) <= size );
  }
  virtual ~Ic9Xd( void ) {}
};

char const * const Ic9Xd::modulations[] = { "FM", "FM-N", "WFM", "AM", "DV" };

class Ic91d : public Ic9Xd {
  Ic91d(             void              );  // Intentionally not implemented
  Ic91d(             Ic91d const & rhs );  // Intentionally not implemented
  Ic91d & operator=( Ic91d const & rhs );  // Intentionally not implemented

 protected:
  virtual char const         * getComment(                      char       * pExport ) const {
    *pExport = 0;
    return "Icom IC-91A/D";
  }
  virtual char const         * setComment(                      char const * pImport )       {
    return "Icom IC-91A/D";
  }

 public:
  Ic91d( char    const * pHeader,
         uint8_t const * pData,
         size_t          size )
      : Ic9Xd( pHeader, pData, size ) {
  }
  virtual ~Ic91d( void ) {}
};

class Ic92d : public Ic9Xd {
  Ic92d(             void              );  // Intentionally not implemented
  Ic92d(             Ic92d const & rhs );  // Intentionally not implemented
  Ic92d & operator=( Ic92d const & rhs );  // Intentionally not implemented

 protected:
  virtual char const         * getComment(                      char       * pExport ) const {
    *pExport = 0;
    return "Icom IC-92AD";
  }
  virtual char const         * setComment(                      char const * pImport )       {
    return "Icom IC-92AD";
  }

 public:
  Ic92d( char    const * pHeader,
         uint8_t const * pData,
         size_t          size )
      : Ic9Xd( pHeader, pData, size ) {
  }
  virtual ~Ic92d( void ) {}
};

Dstar * newIc91d( char    const * pHeader,
                  uint8_t const * pData,
                  size_t          size ) {
  return size == 0xAF90  &&  memcmp( pHeader, "28880000", 8 ) == 0
         ? new Ic91d( pHeader, pData, size ) : 0;
}

Dstar * newIc92d( char    const * pHeader,
                  uint8_t const * pData,
                  size_t          size ) {
  return size == 0xB9F0  &&  memcmp( pHeader, "30660000", 8 ) == 0
         ? new Ic92d( pHeader, pData, size ) : 0;
}
