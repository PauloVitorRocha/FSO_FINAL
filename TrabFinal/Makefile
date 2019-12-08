default:
	cd teste
	find . -type f -exec md5sum "{}" \; > /tmp/checksum
	cd ..
	tar xf teste.bz2.tar
	cd teste.bz2
	find . -type f -exec bunzip2 "{}" \;
	md5sum -c /tmp/checksum
	rm -rf teste.bz2