#
#  $Id: Makefile,v 4.7 2009/04/01 01:20:37 pfeller Exp $
#
#---------------------------------------------------------------------

all:
	./Makeall

perl:
	-(cd perl5; ./setup.sh)

python:
	-(cd python; make)

R:
	-(cd R; cp ../lib/*.[ch] pdq/src; cp ../lib/P*.[h] pdq/lib; R CMD INSTALL pdq)

#---------------------------------------------------------------------

swig:
	-(cd perl5; swig -perl5 -o pdq_wrap.c ../pdq.i)
	-(cd python; swig -python -o pdq_wrap.c ../pdq.i)
	-(cd R; swig -r -o pdq/src/pdq.c ../pdq.i; mv pdq/src/pdq.R pdq/R)

#---------------------------------------------------------------------

test:
	-(cd examples; make test)

#---------------------------------------------------------------------

clean:
	-/bin/rm *~ 
	-(cd lib; make clean)
	-(cd examples; make clean)
	-(cd perl5; make clean)
	-(cd python; make clean)
	-(cd R; make clean)

#---------------------------------------------------------------------

dist:
	-rm -rf /tmp/pdq /tmp/pdq.tar /tmp/pdq.tar.gz
	-mkdir /tmp/pdq
	-cp -r . /tmp/pdq
	-(cd /tmp/pdq; find . -name CVS -exec rm -rf \{\} \;)
	-(cd /tmp/pdq; rm headache.cfg header.txt)
	-(cd /tmp; tar cvf pdq.tar pdq; gzip pdq.tar)
	-rm -rf /tmp/pdq
	-mv /tmp/pdq.tar.gz ..

#---------------------------------------------------------------------

