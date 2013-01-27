

#include <tqobject.h>

#include "kio_man.h"


#include <kapplication.h>
#include <klocale.h>


class kio_man_test : public  MANProtocol
{
  Q_OBJECT

public:
  kio_man_test(const TQCString &pool_socket, const TQCString &app_socket);

protected:
  virtual void data(int);

};





int main(int argc, char **argv)
{
  TDEApplication a( argc, argv , "p2");

  MANProtocol testproto("/tmp/tdeiotest.in", "/tmp/tdeiotest.out");
  testproto.showIndex("3");

  return 0;
}


