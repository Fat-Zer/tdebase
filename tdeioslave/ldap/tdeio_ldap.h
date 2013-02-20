#ifndef __LDAP_H__
#define __LDAP_H__

#include <tqstring.h>
#include <tqvaluelist.h>

#include <tdeio/slavebase.h>
#include <tdeio/authinfo.h>

#define LDAP_DEPRECATED 1 /* Needed for ldap_simple_bind_s with openldap >= 2.3.x */
#include <lber.h>
#include <ldap.h>
#include <tdeabc/ldapurl.h>

class LDAPProtocol : public TDEIO::SlaveBase
{
  public:
    LDAPProtocol( const TQCString &protocol, const TQCString &pool, const TQCString &app );
    virtual ~LDAPProtocol();
    
    virtual void setHost( const TQString& host, int port,
                          const TQString& user, const TQString& pass );

    virtual void openConnection();
    virtual void closeConnection();
    
    virtual void get( const KURL& url );
    virtual void stat( const KURL& url );
    virtual void listDir( const KURL& url );
    virtual void del( const KURL& url, bool isfile );
    virtual void put( const KURL& url, int permissions, bool overwrite, bool resume );

    int saslInteract( void *in );

  private:

    TQString mHost;
    int mPort;
    TQString mUser;
    TQString mPassword;
    LDAP *mLDAP;
    int mVer, mSizeLimit, mTimeLimit;
    bool mTLS;
    bool mAuthSASL;
    TQString mMech,mRealm,mBindName;
    bool mCancel, mFirstAuth;
    
    void controlsFromMetaData( LDAPControl ***serverctrls, 
      LDAPControl ***clientctrls );
    void addControlOp( LDAPControl ***pctrls, const TQString &oid,
      const TQByteArray &value, bool critical );
    void addModOp( LDAPMod ***pmods, int mod_type, 
      const TQString &attr, const TQByteArray &value );
    void LDAPEntry2UDSEntry( const TQString &dn, TDEIO::UDSEntry &entry, 
      const TDEABC::LDAPUrl &usrc, bool dir=false );
    int asyncSearch( TDEABC::LDAPUrl &usrc );
    
    TQCString LDAPEntryAsLDIF( LDAPMessage *msg );
    void LDAPErr( const KURL &url, int err = LDAP_SUCCESS );
    void changeCheck( TDEABC::LDAPUrl &url );

    void fillAuthInfo( TDEIO::AuthInfo &info );
};

#endif
