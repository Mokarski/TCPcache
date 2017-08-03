if autoreconf --install --symlink --force; then
echo "run ./configure"
else
echo "autoreconf fail"
fi