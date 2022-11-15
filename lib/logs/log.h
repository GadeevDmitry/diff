#ifndef LOG_H
#define LOG_H

#define YELLOW "<font color=Gold>"
#define RED    "<font color=DarkRed>"
#define ORANGE "<font color=DarkOrange>"
#define GREEN  "<font color=LimeGreen>"
#define BLUE   "<font color=MediumBlue>"
#define OLIVE  "<font color=Olive>"
#define USUAL  "<font color=Black>"
#define CANCEL "</font>"

/*__________________________________USER_MACRO_DEFINITIONS___________________________________*/

#define log_place()                                                                     \
        log_message("\n"                                                                \
                    "    FILE: %s\n"                                                    \
                    "FUNCTION: %s\n"                                                    \
                    "    LINE: %d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__)

#define log_assert(condition)                                                           \
        if (!(condition))                                                               \
        {                                                                               \
			log_message(RED "ASSERT FAILED: %s\n"                           \
					"         FILE: %s\n"                           \
					"     FUNCTION: %s\n"                           \
					"         LINE: %s\n"           ,               \
				                #condition              ,               \
						__FILE__		,               \
						__PRETTY_FUNCTION__	,               \
						__LINE__		);              \
			abort();                                                        \
        }

/*________________________________USER_FUNCTION_DECLARATIONS_________________________________*/

void    log_message     (const char *fmt, ...);
void    log_error       (const char *fmt, ...);
void    log_warning     (const char *fmt, ...);
void    log_header      (const char *fmt, ...);

void    log_char_ptr    (const char *str_name, const char *str);

void    log_param_place (const char   *file,
                        const char   *func,
                        const int     line);

void   *log_calloc      (size_t number, size_t size);
void    log_free        (void *ptr);
/*___________________________________________________________________________________________*/

#endif //LOG_H