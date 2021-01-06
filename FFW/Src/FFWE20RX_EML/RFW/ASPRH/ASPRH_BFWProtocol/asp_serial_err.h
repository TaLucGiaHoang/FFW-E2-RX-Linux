#ifndef __ASP_SERIAL_ERR_H__
#define __ASP_SERIAL_ERR_H__

// fit SCI
#define BFWERR_SCI_ERR_BAD_CHAN        		(0x3070)    // non-existent channel number
#define BFWERR_SCI_ERR_OMITTED_CHAN    		(0x3071)    // SCI_CHx_INCLUDED is 0 in config.h
#define BFWERR_SCI_ERR_CH_NOT_CLOSED   		(0x3072)    // chan still running in another mode
#define BFWERR_SCI_ERR_BAD_MODE        		(0x3073)    // unsupported or incorrect mode for channel
#define BFWERR_SCI_ERR_INVALID_ARG     		(0x3074)    // argument is not one of the predefined values
#define BFWERR_SCI_ERR_NULL_PTR        		(0x3075)    // received null ptr; missing required argument
#define BFWERR_SCI_ERR_QUEUE_UNAVAILABLE   	(0x3076) 	// can't open tx or rx queue or both
#define BFWERR_SCI_ERR_INSUFFICIENT_SPACE  	(0x3077) 	// not enough space in transmit queue
#define BFWERR_SCI_ERR_INSUFFICIENT_DATA   	(0x3078) 	// not enough data in receive queue

// fit SPI
#define BFWERR_RSPI_ERR_BAD_CHAN       (0x3080)    // Invalid channel number.
#define BFWERR_RSPI_ERR_CH_NOT_OPENED  (0x3081)    // Channel not yet opened.
#define BFWERR_RSPI_ERR_CH_NOT_CLOSED  (0x3082)    // Channel still open from previous open.
#define BFWERR_RSPI_ERR_UNKNOWN_CMD    (0x3083)    // Control command is not recognized.
#define BFWERR_RSPI_ERR_INVALID_ARG    (0x3084)    // Argument is not valid for parameter.
#define BFWERR_RSPI_ERR_ARG_RANGE      (0x3085)    // Argument is out of range for parameter.
#define BFWERR_RSPI_ERR_NULL_PTR       (0x3086)    // Received null pointer; missing required argument.
#define BFWERR_RSPI_ERR_LOCK           (0x3087)    // The lock procedure failed.
#define BFWERR_RSPI_ERR_UNDEF          (0x3088)    // Undefined/unknown error

// fit RIIC
#define BFWERR_RIIC_SUCCESS             (0x3090)    /* Successful operation                                 */
#define BFWERR_RIIC_ERR_LOCK_FUNC       (0x3091)    /* Lock has already been acquired by another task.      */
#define BFWERR_RIIC_ERR_INVALID_CHAN    (0x3092)    /* None existent channel number                         */
#define BFWERR_RIIC_ERR_INVALID_ARG     (0x3093)    /* Parameter error                                      */
#define BFWERR_RIIC_ERR_NO_INIT         (0x3094)    /* Uninitialized state                                  */
#define BFWERR_RIIC_ERR_BUS_BUSY        (0x3095)    /* Channel is on communication.                         */
#define BFWERR_RIIC_ERR_AL              (0x3096)    /* Arbitration lost error                               */
#define BFWERR_RIIC_ERR_TMO             (0x3097)    /* Time Out error                                       */
#define BFWERR_RIIC_ERR_OTHER           (0x3098)    /* Other error                                          */

#endif
