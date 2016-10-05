# Create a directory to download a fresh copy of the project
mkdir project2-submit
# change into it
cd project2-submit
# download project2.git from git
git clone https://github.com/DrY-Courses/OSF16project2.git
# create a list of files to exclude when running diff - diff does not allow "include" it only allows "exclude"
find  OSF16project2/ -type f | grep --text -vP "c$|h$" | sed 's/.*\///'>exclude.files
# create a patch file by diffing your work with the original
diff -aur -x '*~' -x 'examples' -X exclude.files OSF16project2 ../project2 >submit_this_file.patch
# The following two steps are not necessary but done to create a copy of what the TA would actually get when the patch is applied
patch -p0 -b < submit_this_file.patch
# Create an archive of the project as seen by the TA
tar czf project2_as_seen_by_TA.tgz OSF16project2
# cleanup
rm exclude.files
