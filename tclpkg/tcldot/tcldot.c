/* $Id$ $Revision$ */
/* vim:set shiftwidth=4 ts=8: */

/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: See CVS logs. Details at http://www.graphviz.org/
 *************************************************************************/


#include "tcldot.h"

#ifdef WITH_CGRAPH

// Agiddisc functions
static void *myiddisc_open(Agraph_t *g, Agdisc_t *disc) {
	fprintf(stderr,"myiddisc_open:\n");
        return (void *)disc;
}
static long myiddisc_map(void *state, int objtype, char *str, unsigned long *id, int createflag) {
    	mycontext_t *mycontext = (mycontext_t *)state;
	Tcl_Interp *interp = mycontext->interp;
	Tcl_CmdProc *proc = NULL;
	void *tclhandleTblPtr = NULL;
	int rc = 1; // init to success

        switch (objtype) {
                case AGRAPH: tclhandleTblPtr = mycontext->graphTblPtr; proc = graphcmd; break;
                case AGNODE: tclhandleTblPtr = mycontext->nodeTblPtr; proc = nodecmd; break;
                case AGINEDGE:
                case AGOUTEDGE: tclhandleTblPtr = mycontext->edgeTblPtr; proc=edgecmd; break;
        }
	if (createflag) {
		tclhandleAlloc(tclhandleTblPtr, Tcl_GetStringResult(interp), id);
#ifndef TCLOBJ
		Tcl_CreateCommand(interp, Tcl_GetStringResult(interp), proc, (ClientData) mycontext, (Tcl_CmdDeleteProc *) NULL);
#else
		Tcl_CreateObjCommand(interp, Tcl_GetStringResult(interp), proc, (ClientData) mycontext, (Tcl_CmdDeleteProc *) NULL);
#endif           
	}
	else {
		rc = 0;    // FIXME  - not sure about this
	}
        fprintf(stderr,"myiddisc_map: objtype %d, str \"%s\", id %lu, createflag %d, rc = %d\n", objtype, str, *id, createflag, rc);
        return rc;
}
static long myiddisc_alloc(void *state, int objtype, unsigned long id) {
//    	mycontext_t *mycontext = (mycontext_t *)state;

        switch (objtype) {
                case AGRAPH: break;
                case AGNODE: break;
                case AGINEDGE:
                case AGOUTEDGE: break;
        }
        fprintf(stderr,"myiddisc_alloc: objtype %d, id %lu\n", objtype, id);
        return 0;
}
static void myiddisc_free(void *state, int objtype, unsigned long id) {
    	mycontext_t *mycontext = (mycontext_t *)state;
	Tcl_Interp *interp = mycontext->interp;
	char buf[32];
	void *tclhandleTblePtr = NULL;

        switch (objtype) {
                case AGRAPH: tclhandleTblePtr = mycontext->graphTblPtr; break;
                case AGNODE: tclhandleTblePtr = mycontext->nodeTblPtr; break;
                case AGINEDGE:
                case AGOUTEDGE: tclhandleTblePtr = mycontext->edgeTblPtr; break;
        }
	tclhandleString(tclhandleTblePtr, buf, id);
	Tcl_DeleteCommand(interp, buf);
	tclhandleFreeIndex(tclhandleTblePtr, id);
        fprintf(stderr,"myiddisc_free: objtype %d, id %lu\n", objtype, id);
}
static char *myiddisc_print(void *state, int objtype, unsigned long id) {
//    	mycontext_t *mycontext = (mycontext_t *)state;
#if 0
        static char buf[64];
        switch (objtype) {
                case AGRAPH: sprintf(buf, "graph%lu", id); break;
                case AGNODE: sprintf(buf, "node%lu", id); break;
                case AGINEDGE:
                case AGOUTEDGE: sprintf(buf, "edge%lu", id); break;
        }
        fprintf(stderr,"myiddisc_print: objtype %d, id %lu\n", objtype, id);
        return buf;
#else
	return NIL(char*);
#endif
}
static void myiddisc_close(void *state) {
//    	mycontext_t *mycontext = (mycontext_t *)state;
        fprintf(stderr,"myiddisc_close:\n");
}
static Agiddisc_t myiddisc = {
        myiddisc_open,
        myiddisc_map,
        myiddisc_alloc,
        myiddisc_free,
        myiddisc_print,
        myiddisc_close
};

#endif // WITH_CGRAPH

static int dotnew(ClientData clientData, Tcl_Interp * interp,
#ifndef TCLOBJ
		  int argc, char *argv[]
#else				/* TCLOBJ */
		  int argc, Tcl_Obj * CONST objv[]
#endif				/* TCLOBJ */
    )
{
    mycontext_t *mycontext = (mycontext_t *)clientData;
    Agraph_t *g, **gp;
    char c;
    int i, length;
#ifndef WITH_CGRAPH
    int kind;
    unsigned long id;
#else
    Agdesc_t kind;
#endif

    if ((argc < 2)) {
	Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
			 " graphtype ?graphname? ?attributename attributevalue? ?...?\"",
			 NULL);
	return TCL_ERROR;
    }
    c = argv[1][0];
    length = strlen(argv[1]);
    if ((c == 'd') && (strncmp(argv[1], "digraph", length) == 0)) {
#ifndef WITH_CGRAPH
	kind = AGDIGRAPH;
#else
	kind = Agdirected;
#endif
    } else if ((c == 'd')
	       && (strncmp(argv[1], "digraphstrict", length) == 0)) {
#ifndef WITH_CGRAPH
	kind = AGDIGRAPHSTRICT;
#else
	kind = Agstrictdirected;
#endif
    } else if ((c == 'g') && (strncmp(argv[1], "graph", length) == 0)) {
#ifndef WITH_CGRAPH
	kind = AGRAPH;
#else
	kind = Agundirected;
#endif
    } else if ((c == 'g')
	       && (strncmp(argv[1], "graphstrict", length) == 0)) {
#ifndef WITH_CGRAPH
	kind = AGRAPHSTRICT;
#else
	kind = Agstrictundirected;
#endif
    } else {
	Tcl_AppendResult(interp, "bad graphtype \"", argv[1], "\": must be one of:",
			 "\n\tdigraph, digraphstrict, graph, graphstrict.", NULL);
	return TCL_ERROR;
    }
#ifndef WITH_CGRAPH
    gp = (Agraph_t **) tclhandleAlloc(mycontext->graphTblPtr, Tcl_GetStringResult(interp), &id);
#endif
    if (argc % 2) {
	/* if odd number of args then argv[2] is name */
#ifndef WITH_CGRAPH
	g = agopen(argv[2], kind);
#else
	g = agopen(argv[2], kind, (Agdisc_t*)mycontext);
#endif
	i = 3;
    } else {
	/* else use handle as name */
#ifndef WITH_CGRAPH
	g = agopen(Tcl_GetStringResult(interp), kind);
#else
	g = agopen(Tcl_GetStringResult(interp), kind, (Agdisc_t*)mycontext);
#endif
	i = 2;
    }
#ifdef WITH_CGRAPH
    agbindrec(g, "Agraphinfo_t", sizeof(Agraphinfo_t), TRUE);
#endif
    if (!g) {
	Tcl_AppendResult(interp, "\nFailure to open graph.", NULL);
	return TCL_ERROR;
    }
#ifndef WITH_CGRAPH
    *gp = g;
    AGID(g) = id;
#else
    gp = (Agraph_t **)tclhandleXlateIndex(mycontext->graphTblPtr, AGID(g));
    *gp = g;
#endif

#ifndef WITH_CGRAPH
#ifndef TCLOBJ
    Tcl_CreateCommand(interp, Tcl_GetStringResult(interp), graphcmd,
		      (ClientData) mycontext, (Tcl_CmdDeleteProc *) NULL);
#else				/* TCLOBJ */
    Tcl_CreateObjCommand(interp, Tcl_GetStringResult(interp), graphcmd,
			 (ClientData) mycontext, (Tcl_CmdDeleteProc *) NULL);
#endif				/* TCLOBJ */
#endif
    setgraphattributes(g, &argv[i], argc - i);
    /* we use GD_drawing(g) as a flag that layout has been done.
     * so we make sure that it is initialized to "not done" */
    GD_drawing(g) = NULL;

    return TCL_OK;
}

#ifdef WITH_CGRAPH
static void
init_graphs (mycontext_t *mycontext, graph_t* g)
{
    Agraph_t *sg, **sgp;
    unsigned long id;
    char buf[16];
    Tcl_Interp *interp = mycontext->interp;

    for (sg = agfstsubg (g); sg; sg = agnxtsubg (sg))
	init_graphs (mycontext, sg);

    sgp = (Agraph_t **) tclhandleAlloc(mycontext->graphTblPtr, buf, &id);
    *sgp = g;
    AGID(g) = id;
#ifndef TCLOBJ
    Tcl_CreateCommand(interp, buf, graphcmd, (ClientData) mycontext, (Tcl_CmdDeleteProc *) NULL);
#else				/* TCLOBJ */
    Tcl_CreateObjCommand(interp, buf, graphcmd, (ClientData) mycontext, (Tcl_CmdDeleteProc *) NULL);
#endif				/* TCLOBJ */
    if (agroot(g) == g)
	Tcl_SetResult(interp, buf, TCL_VOLATILE);
}
#endif

/*
 * when a graph is read in from a file or string we need to walk
 * it to create the handles and tcl commands for each 
 * graph, subgraph, node, and edge.
 */
static int tcldot_fixup(mycontext_t *mycontext, graph_t * g)
{
#ifndef WITH_CGRAPH
    Agraph_t **gp, *sg, **sgp;
#endif
    Agnode_t *n, **np;
    Agedge_t *e, **ep;
    char buf[16];
    unsigned long id;
    Tcl_Interp *interp = mycontext->interp;

#ifdef WITH_CGRAPH
    init_graphs (mycontext, g);
#else
    if (g->meta_node) {
	for (n = agfstnode(g->meta_node->graph); n;
	     n = agnxtnode(g->meta_node->graph, n)) {
	    sg = agusergraph(n);
	    sgp = (Agraph_t **) tclhandleAlloc(mycontext->graphTblPtr, buf, &id);
	    *sgp = sg;
	    AGID(sg) = id;
#ifndef TCLOBJ
	    Tcl_CreateCommand(interp, buf, graphcmd, (ClientData) mycontext,
			      (Tcl_CmdDeleteProc *) NULL);
#else				/* TCLOBJ */
	    Tcl_CreateObjCommand(interp, buf, graphcmd, (ClientData) mycontext,
				 (Tcl_CmdDeleteProc *) NULL);
#endif				/* TCLOBJ */
	    if (sg == g)
		Tcl_SetResult(interp, buf, TCL_VOLATILE);
	}
    } else {
	gp = (Agraph_t **) tclhandleAlloc(mycontext->graphTblPtr, Tcl_GetStringResult(interp), &id);
	*gp = g;
	AGID(g) = id;
#ifndef TCLOBJ
	Tcl_CreateCommand(interp, Tcl_GetStringResult(interp), graphcmd,
			  (ClientData) mycontext, (Tcl_CmdDeleteProc *) NULL);
#else				/* TCLOBJ */
	Tcl_CreateObjCommand(interp, Tcl_GetStringResult(interp), graphcmd,
			     (ClientData) mycontext, (Tcl_CmdDeleteProc *) NULL);
#endif				/* TCLOBJ */
    }
#endif /* WITH_CGRAPH */
    for (n = agfstnode(g); n; n = agnxtnode(g, n)) {
	np = (Agnode_t **) tclhandleAlloc(mycontext->nodeTblPtr, buf, &id);
	*np = n;
	AGID(n) = id;
#ifndef TCLOBJ
	Tcl_CreateCommand(interp, buf, nodecmd,
			  (ClientData) mycontext, (Tcl_CmdDeleteProc *) NULL);
#else				/* TCLOBJ */
	Tcl_CreateObjCommand(interp, buf, nodecmd,
			     (ClientData) gvc, (Tcl_CmdDeleteProc *) NULL);
#endif				/* TCLOBJ */
	for (e = agfstout(g, n); e; e = agnxtout(g, e)) {
	    ep = (Agedge_t **) tclhandleAlloc(mycontext->edgeTblPtr, buf, &id);
	    *ep = e;
	    AGID(e) = id;
#ifndef TCLOBJ
	    Tcl_CreateCommand(interp, buf, edgecmd, (ClientData) mycontext,
			      (Tcl_CmdDeleteProc *) NULL);
#else				/* TCLOBJ */
	    Tcl_CreateObjCommand(interp, buf, edgecmd, (ClientData) gvc,
				 (Tcl_CmdDeleteProc *) NULL);
#endif				/* TCLOBJ */
	}
    }
    return TCL_OK;
}

/*
 * mygets - same api as gets for libgraph, or read for libcgraph
 *
 * gets one line at a time from a Tcl_Channel and places it in a user buffer
 *    up to a maximum of n characters
 *
 * returns pointer to obtained line in user buffer, or
 * returns NULL when last line read from memory buffer
 *
 * This is probably innefficient because it introduces
 * one more stage of line buffering during reads (at least)
 * but it is needed so that we can take full advantage
 * of the Tcl_Channel mechanism.
 */
#ifdef WITH_CGRAPH
static int mygets(void* channel, char *ubuf, int n)
{
    static Tcl_DString dstr;
    static int strpos;
    int nput;

    if (!n) {			/* a call with n==0 (from aglexinit) resets */
	*ubuf = '\0';
	strpos = 0;
	return 0;
    }

    /* 
     * the user buffer might not be big enough to hold the line.
     */
    if (strpos) {
	nput = Tcl_DStringLength(&dstr) - strpos;
	if (nput > n) {
	    /* chunk between first and last */
	    memcpy(ubuf, (strpos + Tcl_DStringValue(&dstr)), n);
	    strpos += n;
	    nput = n;
	    ubuf[n] = '\0';
	} else {
	    /* last chunk */
	    memcpy(ubuf, (strpos + Tcl_DStringValue(&dstr)), nput);
	    strpos = 0;
	}
    } else {
	Tcl_DStringFree(&dstr);
	Tcl_DStringInit(&dstr);
	if (Tcl_Gets((Tcl_Channel) channel, &dstr) < 0) {
	    /* probably EOF, but could be other read errors */
	    *ubuf = '\0';
	    return 0;
	}
	/* linend char(s) were stripped off by Tcl_Gets,
	 * append a canonical linenend. */
	Tcl_DStringAppend(&dstr, "\n", 1);
	if (Tcl_DStringLength(&dstr) > n) {
	    /* first chunk */
	    nput = n;
	    memcpy(ubuf, Tcl_DStringValue(&dstr), n);
	    strpos = n;
	} else {
	    /* single chunk */
	    nput = Tcl_DStringLength(&dstr);
	    memcpy(ubuf, Tcl_DStringValue(&dstr),nput);
	}
    }
    return nput;
}
#else
static char *mygets(char *ubuf, int n, FILE * channel)
{
    static Tcl_DString dstr;
    static int strpos;

    if (!n) {			/* a call with n==0 (from aglexinit) resets */
	*ubuf = '\0';
	strpos = 0;
	return NULL;
    }

    /* 
     * the user buffer might not be big enough to hold the line.
     */
    if (strpos) {
	if (Tcl_DStringLength(&dstr) > (n + strpos)) {
	    /* chunk between first and last */
	    strncpy(ubuf, (strpos + Tcl_DStringValue(&dstr)), n - 1);
	    strpos += (n - 1);
	    ubuf[n] = '\0';
	} else {
	    /* last chunk */
	    strcpy(ubuf, (strpos + Tcl_DStringValue(&dstr)));
	    strpos = 0;
	}
    } else {
	Tcl_DStringFree(&dstr);
	Tcl_DStringInit(&dstr);
	if (Tcl_Gets((Tcl_Channel) channel, &dstr) < 0) {
	    /* probably EOF, but could be other read errors */
	    *ubuf = '\0';
	    return NULL;
	}
	/* linend char(s) were stripped off by Tcl_Gets,
	 * append a canonical linenend. */
	Tcl_DStringAppend(&dstr, "\n", 1);
	if (Tcl_DStringLength(&dstr) >= n) {
	    /* first chunk */
	    strncpy(ubuf, Tcl_DStringValue(&dstr), n - 1);
	    strpos = n - 1;
	    ubuf[n] = '\0';
	} else {
	    /* single chunk */
	    strcpy(ubuf, Tcl_DStringValue(&dstr));
	}
    }
    return ubuf;

#if 0
    if (!n) {			/* a call with n==0 (from aglexinit) resets */
	mempos = (char *) mbuf;	/* cast from FILE* required by API */
    }

    clp = to = ubuf;
    for (i = 0; i < n - 1; i++) {	/* leave room for terminator */
	if (*mempos == '\0') {
	    if (i) {		/* if mbuf doesn't end in \n, provide one */
		*to++ = '\n';
	    } else {		/* all done */
		clp = NULL;
		mempos = NULL;
	    }
	    break;		/* last line or end-of-buffer */
	}
	if (*mempos == '\n') {
	    *to++ = *mempos++;
	    break;		/* all done with this line */
	}
	*to++ = *mempos++;	/* copy character */
    }
    *to++ = '\0';		/* place terminator in ubuf */
    return clp;
#endif
}
#endif /* WITH_CGRAPH */

#ifdef WITH_CGRAPH

Agraph_t *agread_usergets (FILE * fp, int (*usergets)(void *chan, char *buf, int bufsize))
{
    Agraph_t* g;
    Agdisc_t disc;
    Agiodisc_t ioDisc;

    ioDisc.afread = usergets;
    ioDisc.putstr = AgIoDisc.putstr;
    ioDisc.flush = AgIoDisc.flush;

    disc.mem = &AgMemDisc;
    disc.id = &AgIdDisc;
    disc.io = &ioDisc;
    g = agread (fp, &disc);
    return g;
}
#endif

static int dotread(ClientData clientData, Tcl_Interp * interp,
#ifndef TCLOBJ
		   int argc, char *argv[]
#else				/* TCLOBJ */
		   int argc, Tcl_Obj * CONST objv[]
#endif				/* TCLOBJ */
    )
{
    Agraph_t *g;
    Tcl_Channel channel;
    int mode;
    mycontext_t *mycontext = (mycontext_t *)clientData;

    if (argc < 2) {
	Tcl_AppendResult(interp, "wrong # args: should be \"",
			 argv[0], " fileHandle\"", NULL);
	return TCL_ERROR;
    }
    channel = Tcl_GetChannel(interp, argv[1], &mode);
    if (channel == NULL || !(mode & TCL_READABLE)) {
	Tcl_AppendResult(interp, "\nChannel \"", argv[1], "\"", "is unreadable.", NULL);
	return TCL_ERROR;
    }
    /*
     * read a graph from the channel, the channel is left open
     *   ready to read the first line after the last line of
     *   a properly parsed graph. If the graph doesn't parse
     *   during reading then the channel will be left at EOF
     */
    g = agread_usergets((FILE *) channel, (mygets));
    if (!g) {
	Tcl_AppendResult(interp, "\nFailure to read graph \"", argv[1], "\"", NULL);
	if (agerrors()) {
	    Tcl_AppendResult(interp, " because of syntax errors.", NULL);
	}
	return TCL_ERROR;
    }
    if (agerrors()) {
	Tcl_AppendResult(interp, "\nSyntax errors in file \"", argv[1], " \"", NULL);
	return TCL_ERROR;
    }
    /* we use GD_drawing(g) as a flag that layout has been done.
     * so we make sure that it is initialized to "not done" */
    GD_drawing(g) = NULL;

    return (tcldot_fixup(mycontext, g));
}

static int dotstring(ClientData clientData, Tcl_Interp * interp,
#ifndef TCLOBJ
		     int argc, char *argv[]
#else				/* TCLOBJ */
		     int argc, Tcl_Obj * CONST objv[]
#endif				/* TCLOBJ */
    )
{
    Agraph_t *g;
    mycontext_t *mycontext = (mycontext_t *)clientData;

    if (argc < 2) {
	Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0], " string\"", NULL);
	return TCL_ERROR;
    }
    if (!(g = agmemread(argv[1]))) {
	Tcl_AppendResult(interp, "\nFailure to read string \"", argv[1], "\"", NULL);
	if (agerrors()) {
	    Tcl_AppendResult(interp, " because of syntax errors.", NULL);
	}
	return TCL_ERROR;
    }
    if (agerrors()) {
	Tcl_AppendResult(interp, "\nSyntax errors in string \"", argv[1], " \"", NULL);
	return TCL_ERROR;
    }
    /* we use GD_drawing(g) as a flag that layout has been done.
     * so we make sure that it is initialized to "not done" */
    GD_drawing(g) = NULL;

    return (tcldot_fixup(mycontext, g));
}

#if defined(_BLD_tcldot) && defined(_DLL)
__EXPORT__
#endif
int Tcldot_Init(Tcl_Interp * interp)
{
    mycontext_t *mycontext;
    GVC_t *gvc;

    mycontext = calloc(1, sizeof(mycontext_t));
    if (!mycontext)
	return TCL_ERROR;

    mycontext->interp = interp;
#ifdef WITH_CGRAPH    
    mycontext->mydisc.id = &myiddisc;
#endif

#ifdef USE_TCL_STUBS
    if (Tcl_InitStubs(interp, TCL_VERSION, 0) == NULL) {
	return TCL_ERROR;
    }
#else
    if (Tcl_PkgRequire(interp, "Tcl", TCL_VERSION, 0) == NULL) {
	return TCL_ERROR;
    }
#endif
    if (Tcl_PkgProvide(interp, "Tcldot", VERSION) != TCL_OK) {
	return TCL_ERROR;
    }

#if HAVE_LIBGD
    Gdtclft_Init(interp);
#endif

#ifdef WITH_CGRAPH
    /* set persistent attributes here */
    agattr(NULL, AGNODE, "label", NODENAME_ESC);
#else
    aginit();
    agsetiodisc(NULL, gvfwrite, gvferror);
    /* set persistent attributes here */
    agnodeattr(NULL, "label", NODENAME_ESC);
#endif

    /* create a GraphViz Context and pass a pointer to it in clientdata */
    gvc = gvNEWcontext(lt_preloaded_symbols, DEMAND_LOADING);
    mycontext->gvc = gvc;

    /* configure for available plugins */
    gvconfig(gvc, FALSE);

#ifndef TCLOBJ
    Tcl_CreateCommand(interp, "dotnew", dotnew,
		      (ClientData) mycontext, (Tcl_CmdDeleteProc *) NULL);
    Tcl_CreateCommand(interp, "dotread", dotread,
		      (ClientData) mycontext, (Tcl_CmdDeleteProc *) NULL);
    Tcl_CreateCommand(interp, "dotstring", dotstring,
		      (ClientData) mycontext, (Tcl_CmdDeleteProc *) NULL);
#else				/* TCLOBJ */
    Tcl_CreateObjCommand(interp, "dotnew", dotnew,
			 (ClientData) mycontext, (Tcl_CmdDeleteProc *) NULL);
    Tcl_CreateObjCommand(interp, "dotread", dotread,
			 (ClientData) mycontext, (Tcl_CmdDeleteProc *) NULL);
    Tcl_CreateObjCommand(interp, "dotstring", dotstring,
			 (ClientData) mycontext, (Tcl_CmdDeleteProc *) NULL);
#endif				/* TCLOBJ */

    mycontext->graphTblPtr = (void *) tclhandleInit("graph", sizeof(Agraph_t *), 10);
    mycontext->nodeTblPtr = (void *) tclhandleInit("node", sizeof(Agnode_t *), 100);
    mycontext->edgeTblPtr = (void *) tclhandleInit("edge", sizeof(Agedge_t *), 100);


    return TCL_OK;
}

int Tcldot_SafeInit(Tcl_Interp * interp)
{
    return Tcldot_Init(interp);
}

int Tcldot_builtin_Init(Tcl_Interp * interp)
{
    return Tcldot_Init(interp);
}
