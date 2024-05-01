# Gnu Makefile for B Tree library documentation

PUB-DIR=${HOME}/www-src/hydrus/data/doc
.PHONY:	clean

btlib.html:	btlib.adoc
	asciidoctor  $<

publish:	btlib.html
	cp $< ${PUB-DIR}

clean:
	rm btlib.html asciidoctor.css
