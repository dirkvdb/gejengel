#include <gtest/gtest.h>
#include <fstream>

#include "MusicLibrary/scanner.h"
#include "MusicLibrary/musicdb.h"
#include "MusicLibrary/track.h"
#include "Utils/fileoperations.h"
#include "testfunctions.h"
#include "testclasses.h"

using namespace std;
using namespace FileOperations;
using namespace Gejengel;

#define TEST_DB "test.db"

class ScannerTest : public testing::Test
{
protected:
    virtual void SetUp()
    {
        const char* env = getenv("srcdir");
        ASSERT_FALSE(env == nullptr);
        srcDir = env;
    }

    virtual void TearDown()
    {
        deleteFile(TEST_DB);
    }

    string srcDir;
};

TEST_F(ScannerTest, ScanDirectory)
{
    LibrarySubscriberMock subscriber;
    MusicDb db(TEST_DB);
    db.setSubscriber(subscriber);

<<<<<<< .working
    Scanner scanner(db, subscriber, std::vector<std::string>());
=======
    ScanSubscriberMock scanSubscriber;
    Scanner scanner(db, scanSubscriber, "");
>>>>>>> .merge-right.r571
    scanner.performScan(combinePath(srcDir, "testdata/audio"));

    ASSERT_EQ(3, db.getTrackCount());

    Track item;
    ASSERT_TRUE(db.getTrackWithPath(combinePath(srcDir, "testdata/audio/song1.mp3"), item));

    Track expected = getTrackForSong1();
    item.id = expected.id; //ids can't be guaranteed
    EXPECT_EQ(expected, item);
}
