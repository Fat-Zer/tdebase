#include <tqregexp.h>
#include <tqdir.h>
#include <assert.h>

#include <tdeapplication.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kprocess.h>
#include <tdelocale.h>
#include <tdeconfig.h>


#include "progressdialog.h"
#include "htmlsearch.moc"


HTMLSearch::HTMLSearch()
  : TQObject(), _proc(0)
{
}


TQString HTMLSearch::dataPath(const TQString& _lang)
{
    return kapp->dirs()->saveLocation("data", TQString("khelpcenter/%1").arg(_lang));
}


void HTMLSearch::scanDir(const TQString& dir)
{
    assert( dir.at( dir.length() - 1 ) == '/' );

    TQStringList::ConstIterator it;

    if ( TDEStandardDirs::exists( dir + "index.docbook" ) ) {
        _files.append(dir + "index.docbook");
        progress->setFilesScanned(++_filesScanned);
    } else {
        TQDir d(dir, "*.html", TQDir::Name|TQDir::IgnoreCase, TQDir::Files | TQDir::Readable);
        TQStringList const &list = d.entryList();
        TQString adir = d.canonicalPath () + "/";
        TQString file;
        for (it=list.begin(); it != list.end(); ++it)
        {
            file = adir + *it;
            if ( !_files.contains( file ) ) {
                _files.append(file);
                progress->setFilesScanned(++_filesScanned);
            }
        }
    }

    TQDir d2(dir, TQString::null, TQDir::Name|TQDir::IgnoreCase, TQDir::Dirs);
    TQStringList const &dlist = d2.entryList();
    for (it=dlist.begin(); it != dlist.end(); ++it)
        if (*it != "." && *it != "..")
        {
            scanDir(dir + *it + "/");
            kapp->processEvents();
        }
}


bool HTMLSearch::saveFilesList(const TQString& _lang)
{
    TQStringList dirs;

    // throw away old files list
    _files.clear();

    // open config file
    TDEConfig *config = new TDEConfig("khelpcenterrc");
    config->setGroup("Scope");

    // add KDE help dirs
    if (config->readBoolEntry("KDE", true))
        dirs = kapp->dirs()->findDirs("html", _lang + "/");
    kdDebug() << "got " << dirs.count() << " dirs\n";

    // TODO: Man and Info!!

    // add local urls
    TQStringList add = config->readListEntry("Paths");
    TQStringList::Iterator it;
    for (it = add.begin(); it != add.end(); ++it) {
        if ( ( *it ).at( ( *it ).length() - 1 ) != '/' )
            ( *it ) += '/';
        dirs.append(*it);
    }

    _filesScanned = 0;

    for (it = dirs.begin(); it != dirs.end(); ++it)
        scanDir(*it);

    delete config;

    return true;
}


bool HTMLSearch::createConfig(const TQString& _lang)
{
    TQString fname = dataPath(_lang) + "/htdig.conf";

    // locate the common dir
    TQString wrapper = locate("data", TQString("khelpcenter/%1/wrapper.html").arg(_lang));
    if (wrapper.isEmpty())
        wrapper = locate("data", TQString("khelpcenter/en/wrapper.html"));
    if (wrapper.isEmpty())
        return false;
    wrapper = wrapper.left(wrapper.length() - 12);

    // locate the image dir
    TQString images = locate("data", "khelpcenter/pics/star.png");
    if (images.isEmpty())
        return false;
    images = images.left(images.length() - 8);

    // This is an example replacement for the default bad_words file
    // distributed with ht://Dig. It was compiled by Marjolein Katsma
    // <HSH@taxon.demon.nl>.
    TQString bad_words = i18n( "List of words to exclude from index",
                              "above:about:according:across:actually:\n"
                              "adj:after:afterwards:again:against:all:\n"
                              "almost:alone:along:already:also:although:\n"
                              "always:among:amongst:and:another:any:\n"
                              "anyhow:anyone:anything:anywhere:are:aren:\n"
                              "arent:around:became:because:become:\n"
                              "becomes:becoming:been:before:beforehand:\n"
                              "begin:beginning:behind:being:below:beside:\n"
                              "besides:between:beyond:billion:both:but:\n"
                              "can:cant:cannot:caption:could:couldnt:\n"
                              "did:didnt:does:doesnt:dont:down:during:\n"
                              "each:eight:eighty:either:else:elsewhere:\n"
                              "end:ending:enough:etc:even:ever:every:\n"
                              "everyone:everything:everywhere:except:few:\n"
                              "fifty:first:five:for:former:formerly:forty:\n"
                              "found:four:from:further:had:has:hasnt:have:\n"
                              "havent:hence:her:here:hereafter:hereby:\n"
                              "herein:heres:hereupon:hers:herself:hes:him:\n"
                              "himself:his:how:however:hundred:\n"
                              "inc:indeed:instead:into:isnt:its:\n"
                              "itself:last:later:latter:latterly:least:\n"
                              "less:let:like:likely:ltd:made:make:makes:\n"
                              "many:may:maybe:meantime:meanwhile:might:\n"
                              "million:miss:more:moreover:most:mostly:\n"
                              "mrs:much:must:myself:namely:neither:\n"
                              "never:nevertheless:next:nine:ninety:\n"
                              "nobody:none:nonetheless:noone:nor:not:\n"
                              "nothing:now:nowhere:off:often:once:\n"
                              "one:only:onto:others:otherwise:our:ours:\n"
                              "ourselves:out:over:overall:own:page:per:\n"
                              "perhaps:rather:recent:recently:same:\n"
                              "seem:seemed:seeming:seems:seven:seventy:\n"
                              "several:she:shes:should:shouldnt:since:six:\n"
                              "sixty:some:somehow:someone:something:\n"
                              "sometime:sometimes:somewhere:still:stop:\n"
                              "such:taking:ten:than:that:the:their:them:\n"
                              "themselves:then:thence:there:thereafter:\n"
                              "thereby:therefore:therein:thereupon:these:\n"
                              "they:thirty:this:those:though:thousand:\n"
                              "three:through:throughout:thru:thus:tips:\n"
                              "together:too:toward:towards:trillion:\n"
                              "twenty:two:under:unless:unlike:unlikely:\n"
                              "until:update:updated:updates:upon:\n"
                              "used:using:very:via:want:wanted:wants:\n"
                              "was:wasnt:way:ways:wed:well:were:\n"
                              "werent:what:whats:whatever:when:whence:\n"
                              "whenever:where:whereafter:whereas:whereby:\n"
                              "wherein:whereupon:wherever:wheres:whether:\n"
                              "which:while:whither:who:whoever:whole:\n"
                              "whom:whomever:whose:why:will:with:within:\n"
                              "without:wont:work:worked:works:working:\n"
                              "would:wouldnt:yes:yet:you:youd:youll:your:\n"
                              "youre:yours:yourself:yourselves:youve" );

    TQFile f;
    f.setName( dataPath(_lang) + "/bad_words" );
    if (f.open(IO_WriteOnly))
    {
        TQTextStream ts( &f );
        TQStringList words = TQStringList::split ( TQRegExp ( "[\n:]" ),
                                                 bad_words, false);
        for ( TQStringList::ConstIterator it = words.begin();
              it != words.end(); ++it )
            ts << *it << endl;
        f.close();
    }

    f.setName(fname);
    if (f.open(IO_WriteOnly))
    {
        kdDebug() << "Writing config for " << _lang << " to " << fname << endl;

        TQTextStream ts(&f);

        ts << "database_dir:\t\t" << dataPath(_lang) << endl;
        ts << "start_url:\t\t`" << dataPath(_lang) << "/files`" << endl;
        ts << "local_urls:\t\tfile:/=/" << endl;
        ts << "local_urls_only:\ttrue" << endl;
        ts << "maximum_pages:\t\t1" << endl;
        ts << "image_url_prefix:\t" << images << endl;
        ts << "star_image:\t\t" << images << "star.png" << endl;
        ts << "star_blank:\t\t" << images << "star_blank.png" << endl;
        ts << "compression_level:\t6" << endl;
        ts << "max_hop_count:\t\t0" << endl;

        ts << "search_results_wrapper:\t" << wrapper << "wrapper.html" << endl;
        ts << "nothing_found_file:\t" << wrapper << "nomatch.html" << endl;
        ts << "syntax_error_file:\t" << wrapper << "syntax.html" << endl;
        ts << "bad_word_list:\t\t" << dataPath(_lang) << "/bad_words" << endl;
        ts << "external_parsers:\t" << "text/xml\t" << locate( "data", "khelpcenter/meinproc_wrapper" ) << endl;
        f.close();
        return true;
    }

    return false;
}


#define CHUNK_SIZE 15

bool HTMLSearch::generateIndex(TQString _lang, TQWidget *parent)
{
    if (_lang == "C")
        _lang = "en";

    if (!createConfig(_lang))
        return false;

    // create progress dialog
    progress = new ProgressDialog(parent);
    progress->show();
    kapp->processEvents();

    // create files list ----------------------------------------------
    if (!saveFilesList(_lang))
        return false;

    progress->setState(1);

    // run htdig ------------------------------------------------------
    TDEConfig *config = new TDEConfig("khelpcenterrc", true);
    TDEConfigGroupSaver saver(config, "htdig");
    TQString exe = config->readPathEntry("htdig", kapp->dirs()->findExe("htdig"));

    if (exe.isEmpty())
    {
        delete config;
        return false;
    }
    bool initial = true;
    bool done = false;
    int  count = 0;

    _filesToDig = _files.count();
    progress->setFilesToDig(_filesToDig);
    _filesDigged = 0;

    TQDir d; d.mkdir(dataPath(_lang));

    while (!done)
    {
        // kill old process
        delete _proc;

        // prepare new process
        _proc = new TDEProcess();
        *_proc << exe  << "-v" << "-c" << dataPath(_lang)+"/htdig.conf";
        if (initial)
	{
            *_proc << "-i";
            initial = false;
	}

        kdDebug() << "Running htdig" << endl;

        connect(_proc, TQT_SIGNAL(receivedStdout(TDEProcess *,char*,int)),
                this, TQT_SLOT(htdigStdout(TDEProcess *,char*,int)));

        connect(_proc, TQT_SIGNAL(processExited(TDEProcess *)),
                this, TQT_SLOT(htdigExited(TDEProcess *)));

        _htdigRunning = true;

        // write out file
        TQFile f(dataPath(_lang)+"/files");
        if (f.open(IO_WriteOnly))
	{
            TQTextStream ts(&f);

            for (int i=0; i<CHUNK_SIZE; ++i, ++count)
                if (count < _filesToDig) {
                    ts << "file://" + _files[count] << endl;
                } else {
                    done = true;
                    break;
                }
            f.close();
	}
        else
	{
            kdDebug() << "Could not open `files` for writing" << endl;
            delete config;
            return false;
	}


        // execute htdig
        _proc->start(TDEProcess::NotifyOnExit, TDEProcess::Stdout );

        kapp->enter_loop();

        if (!_proc->normalExit() || _proc->exitStatus() != 0)
	{
            delete _proc;
            delete progress;
            delete config;
            return false;
	}

        // _filesDigged += CHUNK_SIZE;
        progress->setFilesDigged(_filesDigged);
        kapp->processEvents();
    }

    progress->setState(2);

    // run htmerge -----------------------------------------------------
    exe = config->readPathEntry("htmerge", kapp->dirs()->findExe("htmerge"));
    if (exe.isEmpty())
    {
        delete config;
        return false;
    }
    delete _proc;
    _proc = new TDEProcess();
    *_proc << exe << "-c" << dataPath(_lang)+"/htdig.conf";

    kdDebug() << "Running htmerge" << endl;

    connect(_proc, TQT_SIGNAL(processExited(TDEProcess *)),
            this, TQT_SLOT(htmergeExited(TDEProcess *)));

    _htmergeRunning = true;

    _proc->start(TDEProcess::NotifyOnExit, TDEProcess::Stdout);

    kapp->enter_loop();

    if (!_proc->normalExit() || _proc->exitStatus() != 0)
    {
        delete _proc;
        delete progress;
        delete config;
        return false;
    }

    delete _proc;

    progress->setState(3);
    kapp->processEvents();

    delete progress;
    delete config;

    return true;
}



void HTMLSearch::htdigStdout(TDEProcess *, char *buffer, int len)
{
    TQString line = TQString(buffer).left(len);

    int cnt=0, index=-1;
    while ( (index = line.find("file://", index+1)) > 0)
        cnt++;
    _filesDigged += cnt;

    cnt=0;
    index=-1;
    while ( (index = line.find("not changed", index+1)) > 0)
        cnt++;
    _filesDigged -= cnt;

    progress->setFilesDigged(_filesDigged);
}


void HTMLSearch::htdigExited(TDEProcess *p)
{
    kdDebug() << "htdig terminated " << p->exitStatus() << endl;
    _htdigRunning = false;
    kapp->exit_loop();
}


void HTMLSearch::htmergeExited(TDEProcess *)
{
  kdDebug() << "htmerge terminated" << endl;
  _htmergeRunning = false;
  kapp->exit_loop();
}


void HTMLSearch::htsearchStdout(TDEProcess *, char *buffer, int len)
{
  _searchResult += TQString::fromLocal8Bit(buffer,len);
}


void HTMLSearch::htsearchExited(TDEProcess *)
{
  kdDebug() << "htsearch terminated" << endl;
  _htsearchRunning = false;
  kapp->exit_loop();
}


TQString HTMLSearch::search(TQString _lang, TQString words, TQString method, int matches,
			   TQString format, TQString sort)
{
  if (_lang == "C")
    _lang = "en";

  createConfig(_lang);

  TQString result = dataPath(_lang)+"/result.html";

  // run htsearch ----------------------------------------------------
  TDEConfig *config = new TDEConfig("khelpcenterrc", true);
  TDEConfigGroupSaver saver(config, "htdig");
  TQString exe = config->readPathEntry("htsearch", kapp->dirs()->findExe("htsearch"));
  if (exe.isEmpty())
  {
      delete config;
    return TQString::null;
  }
  _proc = new TDEProcess();
  *_proc << exe << "-c" << dataPath(_lang)+"/htdig.conf" <<
    TQString("words=%1;method=%2;matchesperpage=%3;format=%4;sort=%5").arg(words).arg(method).arg(matches).arg(format).arg(sort);

  kdDebug() << "Running htsearch" << endl;

  connect(_proc, TQT_SIGNAL(receivedStdout(TDEProcess *,char*,int)),
	  this, TQT_SLOT(htsearchStdout(TDEProcess *,char*,int)));
  connect(_proc, TQT_SIGNAL(processExited(TDEProcess *)),
	  this, TQT_SLOT(htsearchExited(TDEProcess *)));

  _htsearchRunning = true;
  _searchResult = "";

  _proc->start(TDEProcess::NotifyOnExit, TDEProcess::Stdout);

  kapp->enter_loop();

  if (!_proc->normalExit() || _proc->exitStatus() != 0)
    {
      kdDebug() << "Error running htsearch... returning now" << endl;
      delete _proc;
      delete config;
      return TQString::null;
    }

  delete _proc;

  // modify the search result
  _searchResult = _searchResult.replace("http://localhost/", "file:/");
  _searchResult = _searchResult.replace("Content-type: text/html", TQString::null);

  // dump the search result
  TQFile f(result);
  if (f.open(IO_WriteOnly))
    {
      TQTextStream ts(&f);

      ts << _searchResult << endl;

      f.close();
      delete config;
      return result;
    }
  delete config;
  return TQString::null;
}
