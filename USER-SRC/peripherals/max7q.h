#ifndef MAX7Q_H
#define MAX7Q_H

#include "pipe_hal.h"
#include "serial_pipe_hal.h"

/** basic gps parser class 
*/
class GPSParser
{
public:
    /** Power on / Wake up the gps 
    */
    virtual bool init(/*PinName pn - not use */) = 0;
    
    enum { 
        // getLine Responses
        WAIT      = -1, //!< wait for more incoming data (the start of a message was found, or no data available)
        NOT_FOUND =  0, //!< a parser concluded the the current offset of the pipe doe not contain a valid message
    
        #define LENGTH(x)   (x & 0x00FFFF)  //!< extract/mask the length
        #define PROTOCOL(x) (x & 0xFF0000)  //!< extract/mask the type

        UNKNOWN   = 0x000000,       //!< message type is unknown 
        UBX       = 0x100000,       //!< message if of protocol NMEA
        NMEA      = 0x200000        //!< message if of protocol UBX
    };
    
    /** Get a line from the physical interface. This function
        needs to be implemented in the inherited class.
        \param buf the buffer to store it
        \param len size of the buffer
        \return type and length if something was found, 
                WAIT if not enough data is available
                NOT_FOUND if nothing was found
    */ 
    virtual int getMessage(char* buf, int len) = 0;
    
    /** send a buffer
        \param buf the buffer to write
        \param len size of the buffer to write
        \return bytes written
    */
    virtual int send(const char* buf, int len);
    
    /** send a NMEA message, this function just takes the 
        payload and calculates and adds checksum. ($ and *XX\r\n will be added)
        \param buf the message payload to write
        \param len size of the message payload to write
        \return total bytes written
    */
    virtual int sendNmea(const char* buf, int len);
    
    /** send a UBX message, this function just takes the 
        payload and calculates and adds checksum.
        \param cls the UBX class id 
        \param id the UBX message id
        \param buf the message payload to write
        \param len size of the message payload to write
        \return total bytes written
    */
    virtual int sendUbx(unsigned char cls, unsigned char id, 
                        const void* buf = NULL, int len = 0);
    
    /** Power off the gps, it can be again woken up by an 
        edge on the serial port on the external interrupt pin. 
    */
    void powerOff(void);
    
    /** get the first character of a NMEA field
        \param ix the index of the field to find
        \param start the start of the buffer
        \param end the end of the buffer
        \return the pointer to the first character of the field. 
    */
    static const char* findNmeaItemPos(int ix, const char* start, const char* end);
    
    /** extract a double value from a buffer containing a NMEA message
        \param ix the index of the field to extract
        \param buf the NMEA message
        \param len the size of the NMEA message
        \param val the extracted value
        \return true if successful, false otherwise
    */
    static bool getNmeaItem(int ix, char* buf, int len, double& val);
    
    /** extract a interger value from a buffer containing a NMEA message
        \param ix the index of the field to extract
        \param buf the NMEA message
        \param len the size of the NMEA message
        \param val the extracted value
        \param base the numeric base to be used (e.g. 8, 10 or 16)
        \return true if successful, false otherwise
    */
    static bool getNmeaItem(int ix, char* buf, int len, int& val, int base/*=10*/);
    
    /** extract a char value from a buffer containing a NMEA message
        \param ix the index of the field to extract
        \param buf the NMEA message
        \param len the size of the NMEA message
        \param val the extracted value
        \return true if successful, false otherwise
    */
    static bool getNmeaItem(int ix, char* buf, int len, char& val);
    
    /** extract a latitude/longitude value from a buffer containing a NMEA message
        \param ix the index of the field to extract (will extract ix and ix + 1)
        \param buf the NMEA message
        \param len the size of the NMEA message
        \param val the extracted latitude or longitude
        \return true if successful, false otherwise
    */
    static bool getNmeaAngle(int ix, char* buf, int len, double& val);
    
protected:
    /** Get a line from the physical interface. 
        \param pipe the receiveing pipe to parse messages 
        \param buf the buffer to store it
        \param len size of the buffer
        \return type and length if something was found, 
                WAIT if not enough data is available
                NOT_FOUND if nothing was found
    */ 
    static int _getMessage(Pipe<char>* pipe, char* buf, int len);
    
    /** Check if the current offset of the pipe contains a NMEA message.
        \param pipe the receiveing pipe to parse messages 
        \param len numer of bytes to parse at maximum
        \return length if something was found (including the NMEA frame) 
                WAIT if not enough data is available
                NOT_FOUND if nothing was found
    */ 
    static int _parseNmea(Pipe<char>* pipe, int len);
    
    /** Check if the current offset of the pipe contains a UBX message.
        \param pipe the receiveing pipe to parse messages 
        \param len numer of bytes to parse at maximum
        \return length if something was found (including the UBX frame)
                WAIT if not enough data is available
                NOT_FOUND if nothing was found
    */ 
    static int _parseUbx(Pipe<char>* pipe, int len);
    
    /** Write bytes to the physical interface. This function 
        needs to be implemented by the inherited class. 
        \param buf the buffer to write
        \param len size of the buffer to write
        \return bytes written
    */
    virtual int _send(const void* buf, int len) = 0;
    
    static const char toHex[16]; //!< num to hex conversion

};

/** gps class which uses a serial port 
    as physical interface. 
*/
class GPSSerial : public SerialPipe, public GPSParser
{
public:
    /** Constructor
        \param rxSize the size of the serial rx buffer
    */         
    GPSSerial(int rxSize = 256);
    //! Destructor
    virtual ~GPSSerial(void);
    
    virtual bool init(/*PinName pn = NC*/);
    
    /** Get a line from the physical interface. 
        \param buf the buffer to store it
        \param len size of the buffer
        \return type and length if something was found, 
                WAIT if not enough data is available
                NOT_FOUND if nothing was found
    */ 
    virtual int getMessage(char* buf, int len);
    
protected:
    /** Write bytes to the physical interface.
        \param buf the buffer to write
        \param len size of the buffer to write
        \return bytes written
    */
    virtual int _send(const void* buf, int len);
		
		bool putc(int c)     { char ch = c; return send(&ch, 1);}

};

extern GPSSerial gnss;

#endif

