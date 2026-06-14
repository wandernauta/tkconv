#include "stemmer.hh"

#include <libstemmer.h>
#include <sqlite3.h>

// FTS5 tokenizer die ook woorden reduceert tot hun stam, bijvoorbeeld
// 'besluiten' naar 'besluit', zodat om het even welke kan worden gebruikt in
// zoekvragen.
//
// De truc om een bestaande (waarschijnlijk: unicode61) tokenizer te gebruiken
// voor het daadwerkelijke tokenizen is afgekeken van de ingebouwde "porter"
// stemmer. Gebruik als volgt:
//
// CREATE VIRTUAL TABLE ... USING fts5(..., tokenize="dutch unicode61 ...")

struct DutchTokenizer {
  fts5_tokenizer_v2 *delegateModule;
  Fts5Tokenizer *delegateInstance;
  sb_stemmer *stemmer;
};

struct DutchTokenizerContext {
  void *delegateContext;
  int (*originalCb)(void*, int, const char*, int, int, int);
  sb_stemmer *stemmer;
};

static int dutchCreate(void* ctx, const char **args, int nArg, Fts5Tokenizer **out) {
  int rc = 0;
  DutchTokenizer *tok = new DutchTokenizer{};
  *out = (Fts5Tokenizer*)tok;

  fts5_api *api = (fts5_api*)ctx;
  
  void *userdata = 0;

  if (nArg < 1)
    return SQLITE_MISUSE;

  if ((rc = api->xFindTokenizer_v2(api, args[0], &userdata, &tok->delegateModule)))
    return rc;

  if ((rc = tok->delegateModule->xCreate(userdata, args + 1, nArg - 1, &tok->delegateInstance)))
    return rc;

  if ((tok->stemmer = sb_stemmer_new("dutch", nullptr)) == nullptr)
    return SQLITE_NOMEM;

  return rc;
}

static void dutchDelete(Fts5Tokenizer* tok) {
  DutchTokenizer *self = (DutchTokenizer*)tok;

  if (self) {
    if (self->delegateModule) {
      self->delegateModule->xDelete(self->delegateInstance);
    }
    if (self->stemmer) {
      sb_stemmer_delete(self->stemmer);
    }
    delete self;
  }
}

static int dutchToken(void* p, int tflags, const char* buf, int len, int start, int end) {
  DutchTokenizerContext *ctx = (DutchTokenizerContext*)p;

  const char* stemmedSym = (const char*)sb_stemmer_stem(ctx->stemmer, (sb_symbol*)buf, len);
  int stemmedLen = sb_stemmer_length(ctx->stemmer);

  return ctx->originalCb(ctx->delegateContext, tflags, stemmedSym, stemmedLen, start, end);
}

static int dutchTokenize(Fts5Tokenizer *tok, void *pCtx, int flags, const char *text, int t, const char *locale, int l, int (*token)(void*, int, const char*, int, int, int)) {
  DutchTokenizer *self = (DutchTokenizer*)tok;

  DutchTokenizerContext ctx = {
    .delegateContext = pCtx,
    .originalCb = token,
    .stemmer = self->stemmer,
  };

  return self->delegateModule->xTokenize(self->delegateInstance, &ctx, flags, text, t, locale, l, dutchToken);
}

static fts5_tokenizer_v2 dutch = {
  .iVersion = 2,
  .xCreate = dutchCreate,
  .xDelete = dutchDelete,
  .xTokenize = dutchTokenize,
};

int installDutch(sqlite3 *db, char **pzErrMsg, const struct sqlite3_api_routines *pThunk) {
  int rc = 0;
  sqlite3_stmt *stmt = nullptr;
  fts5_api *fts5 = nullptr;
  
  if ((rc = sqlite3_prepare(db, "SELECT fts5(?)", -1, &stmt, nullptr)))
    return rc;

  sqlite3_bind_pointer(stmt, 1, (void*)&fts5, "fts5_api_ptr", nullptr);
  sqlite3_step(stmt);

  if ((rc = sqlite3_finalize(stmt)))
    return rc;

  return fts5->xCreateTokenizer_v2(fts5, "dutch", fts5, &dutch, nullptr);
}
