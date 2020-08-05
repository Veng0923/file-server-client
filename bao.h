#define STATUS_FILE 1
#define STATUS_END 10
struct Protocol
{
  int status;
  char body[2048];
  int size;
};
