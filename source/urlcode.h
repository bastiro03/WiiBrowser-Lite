/* Returns a url-encoded version of str */
/* IMPORTANT: be sure to free() the returned string after use */
extern char* url_encode(char* str);

/* Returns a url-encoded (spaces only) version of str */
/* IMPORTANT: be sure to free() the returned string after use */
extern char* url_encode_lite(char* str);

/* Returns a url-decoded version of str */
/* IMPORTANT: be sure to free() the returned string after use */
extern char* url_decode(char* str);
