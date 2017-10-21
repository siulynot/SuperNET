
/******************************************************************************
 * Copyright © 2014-2017 The SuperNET Developers.                             *
 *                                                                            *
 * See the AUTHORS, DEVELOPER-AGREEMENT and LICENSE files at                  *
 * the top-level directory of this distribution for the individual copyright  *
 * holder information and the developer policies on copyright and licensing.  *
 *                                                                            *
 * Unless otherwise agreed in a custom licensing agreement, no part of the    *
 * SuperNET software, including this file may be copied, modified, propagated *
 * or distributed except according to the terms contained in the LICENSE file *
 *                                                                            *
 * Removal or modification of this copyright notice is prohibited.            *
 *                                                                            *
 ******************************************************************************/

//
//  LP_signatures.c
//  marketmaker
//

struct basilisk_request *LP_requestinit(struct basilisk_request *rp,bits256 srchash,bits256 desthash,char *src,uint64_t srcsatoshis,char *dest,uint64_t destsatoshis,uint32_t timestamp,uint32_t quotetime,int32_t DEXselector)
{
    struct basilisk_request R;
    memset(rp,0,sizeof(*rp));
    rp->srchash = srchash;
    rp->srcamount = srcsatoshis;
    rp->timestamp = timestamp;
    rp->DEXselector = DEXselector;
    safecopy(rp->src,src,sizeof(rp->src));
    safecopy(rp->dest,dest,sizeof(rp->dest));
    R = *rp;
    rp->requestid = basilisk_requestid(rp);
    rp->quotetime = quotetime;
    rp->desthash = desthash;
    rp->destamount = destsatoshis;
    rp->quoteid = basilisk_quoteid(rp);
    printf("r.%u %u, q.%u %u: %s %.8f -> %s %.8f\n",rp->timestamp,rp->requestid,rp->quotetime,rp->quoteid,rp->src,dstr(rp->srcamount),rp->dest,dstr(rp->destamount));
    return(rp);
}

cJSON *LP_quotejson(struct LP_quoteinfo *qp)
{
    double price; cJSON *retjson = cJSON_CreateObject();
    jaddstr(retjson,"base",qp->srccoin);
    jaddstr(retjson,"rel",qp->destcoin);
    if ( qp->coinaddr[0] != 0 )
        jaddstr(retjson,"address",qp->coinaddr);
    if ( qp->timestamp != 0 )
        jaddnum(retjson,"timestamp",qp->timestamp);
    if ( bits256_nonz(qp->txid) != 0 )
    {
        jaddbits256(retjson,"txid",qp->txid);
        jaddnum(retjson,"vout",qp->vout);
    }
    if ( bits256_nonz(qp->srchash) != 0 )
        jaddbits256(retjson,"srchash",qp->srchash);
    if ( qp->txfee != 0 )
        jadd64bits(retjson,"txfee",qp->txfee);
    if ( qp->quotetime != 0 )
        jaddnum(retjson,"quotetime",qp->quotetime);
    if ( qp->satoshis != 0 )
        jadd64bits(retjson,"satoshis",qp->satoshis);
    if ( bits256_nonz(qp->desthash) != 0 )
        jaddbits256(retjson,"desthash",qp->desthash);
    if ( bits256_nonz(qp->txid2) != 0 )
    {
        jaddbits256(retjson,"txid2",qp->txid2);
        jaddnum(retjson,"vout2",qp->vout2);
    }
    if ( bits256_nonz(qp->desttxid) != 0 )
    {
        if ( qp->destaddr[0] != 0 )
            jaddstr(retjson,"destaddr",qp->destaddr);
        jaddbits256(retjson,"desttxid",qp->desttxid);
        jaddnum(retjson,"destvout",qp->destvout);
    }
    if ( bits256_nonz(qp->feetxid) != 0 )
    {
        jaddbits256(retjson,"feetxid",qp->feetxid);
        jaddnum(retjson,"feevout",qp->feevout);
    }
    if ( qp->desttxfee != 0 )
        jadd64bits(retjson,"desttxfee",qp->desttxfee);
    if ( qp->destsatoshis != 0 )
    {
        jadd64bits(retjson,"destsatoshis",qp->destsatoshis);
        if ( qp->satoshis != 0 )
        {
            price = (double)qp->destsatoshis / (qp->satoshis - qp->txfee);
            jaddnum(retjson,"price",price);
        }
    }
    return(retjson);
}

int32_t LP_quoteparse(struct LP_quoteinfo *qp,cJSON *argjson)
{
    safecopy(qp->srccoin,jstr(argjson,"base"),sizeof(qp->srccoin));
    safecopy(qp->coinaddr,jstr(argjson,"address"),sizeof(qp->coinaddr));
    safecopy(qp->destcoin,jstr(argjson,"rel"),sizeof(qp->destcoin));
    safecopy(qp->destaddr,jstr(argjson,"destaddr"),sizeof(qp->destaddr));
    qp->timestamp = juint(argjson,"timestamp");
    qp->quotetime = juint(argjson,"quotetime");
    qp->txid = jbits256(argjson,"txid");
    qp->txid2 = jbits256(argjson,"txid2");
    qp->vout = jint(argjson,"vout");
    qp->vout2 = jint(argjson,"vout2");
    qp->feevout = jint(argjson,"feevout");
    qp->srchash = jbits256(argjson,"srchash");
    qp->desttxid = jbits256(argjson,"desttxid");
    qp->feetxid = jbits256(argjson,"feetxid");
    qp->destvout = jint(argjson,"destvout");
    qp->desthash = jbits256(argjson,"desthash");
    qp->satoshis = j64bits(argjson,"satoshis");
    qp->destsatoshis = j64bits(argjson,"destsatoshis");
    qp->txfee = j64bits(argjson,"txfee");
    qp->desttxfee = j64bits(argjson,"desttxfee");
    return(0);
}

void LP_txfees(uint64_t *txfeep,uint64_t *desttxfeep,char *base,char *rel)
{
    *txfeep = LP_txfeecalc(LP_coinfind(base),0,0);
    *desttxfeep = LP_txfeecalc(LP_coinfind(rel),0,0);
    printf("LP_txfees(%.8f %.8f)\n",dstr(*txfeep),dstr(*desttxfeep));
}

int32_t LP_quoteinfoinit(struct LP_quoteinfo *qp,struct LP_utxoinfo *utxo,char *destcoin,double price,uint64_t satoshis,uint64_t destsatoshis)
{
    memset(qp,0,sizeof(*qp));
    if ( qp->timestamp == 0 )
        qp->timestamp = (uint32_t)time(NULL);
    safecopy(qp->destcoin,destcoin,sizeof(qp->destcoin));
    LP_txfees(&qp->txfee,&qp->desttxfee,utxo->coin,qp->destcoin);
    qp->satoshis = satoshis;//(destsatoshis / price) + 0.49;
    qp->destsatoshis = destsatoshis;
    /*if ( qp->txfee >= qp->satoshis || qp->txfee >= utxo->deposit.value || utxo->deposit.value < LP_DEPOSITSATOSHIS(qp->satoshis) ) //utxo->iambob == 0 ||
    {
        printf("quoteinit error.(%d %d %d %d) %.8f vs %.8f\n",utxo->iambob == 0,qp->txfee >= qp->satoshis,qp->txfee >= utxo->deposit.value,utxo->deposit.value < LP_DEPOSITSATOSHIS(qp->satoshis),dstr(utxo->deposit.value),dstr(LP_DEPOSITSATOSHIS(qp->satoshis)));
        return(-1);
    }*/
    qp->txid = utxo->payment.txid;
    qp->vout = utxo->payment.vout;
    qp->txid2 = utxo->deposit.txid;
    qp->vout2 = utxo->deposit.vout;
    if ( qp->desttxfee >= qp->destsatoshis )
    {
        printf("quoteinit desttxfee %.8f < %.8f destsatoshis\n",dstr(qp->desttxfee),dstr(qp->destsatoshis));
        return(-2);
    }
    safecopy(qp->srccoin,utxo->coin,sizeof(qp->srccoin));
    safecopy(qp->coinaddr,utxo->coinaddr,sizeof(qp->coinaddr));
    qp->srchash = utxo->pubkey;
    return(0);
}

int32_t LP_quotedestinfo(struct LP_quoteinfo *qp,bits256 desttxid,int32_t destvout,bits256 feetxid,int32_t feevout,bits256 desthash,char *destaddr)
{
    qp->desttxid = desttxid;
    qp->destvout = destvout;
    qp->desthash = desthash;
    qp->feetxid = feetxid;
    qp->feevout = feevout;
    safecopy(qp->destaddr,destaddr,sizeof(qp->destaddr));
    return(0);
}

char *LP_quotereceived(cJSON *argjson)
{
    struct LP_cacheinfo *ptr; double price; struct LP_quoteinfo Q;
    LP_quoteparse(&Q,argjson);
    price = (double)Q.destsatoshis / (Q.satoshis - Q.txfee);
    if ( (ptr= LP_cacheadd(Q.srccoin,Q.destcoin,Q.txid,Q.vout,price,&Q)) != 0 )
    {
        ptr->Q = Q;
        printf(">>>>>>>>>> received quote %s/%s %.8f\n",Q.srccoin,Q.destcoin,price);
        return(clonestr("{\"result\":\"updated\"}"));
    } else return(clonestr("{\"error\":\"nullptr\"}"));
}

void LP_smartutxos_push(struct iguana_info *coin)
{
    struct LP_peerinfo *peer,*tmp; uint64_t value; bits256 zero,txid; int32_t i,vout,height,n; char *retstr; cJSON *array,*item,*req;
    if ( coin->smartaddr[0] == 0 )
        return;
    if ( (array= LP_address_utxos(coin,coin->smartaddr,1)) != 0 )
    {
        memset(zero.bytes,0,sizeof(zero));
        if ( (n= cJSON_GetArraySize(array)) > 0 )
        {
            //printf("PUSH %s %s\n",coin->symbol,coin->smartaddr);
            for (i=0; i<n; i++)
            {
                item = jitem(array,i);
                txid = jbits256(item,"tx_hash");
                vout = jint(item,"tx_pos");
                value = j64bits(item,"value");
                height = jint(item,"height");
                if ( 0 && (rand() % 100) == 0 && IAMLP == 0 )
                {
                    HASH_ITER(hh,LP_peerinfos,peer,tmp)
                    {
                        if ( (retstr= issue_LP_uitem(peer->ipaddr,peer->port,coin->symbol,coin->smartaddr,txid,vout,height,value)) != 0 )
                            free(retstr);
                    }
                }
                req = cJSON_CreateObject();
                jaddstr(req,"method","uitem");
                jaddstr(req,"coin",coin->symbol);
                jaddstr(req,"coinaddr",coin->smartaddr);
                jaddbits256(req,"txid",txid);
                jaddnum(req,"vout",vout);
                jaddnum(req,"ht",height);
                jadd64bits(req,"value",value);
                //printf("ADDR_UNSPENTS[] <- %s\n",jprint(req,0));
                LP_reserved_msg("","",zero,jprint(req,1));
            }
        }
        free_json(array);
    }
}

char *LP_uitem_recv(cJSON *argjson)
{
    bits256 txid; int32_t vout,height; uint64_t value; char *coinaddr,*symbol;
    txid = jbits256(argjson,"txid");
    vout = jint(argjson,"vout");
    height = jint(argjson,"ht");
    value = j64bits(argjson,"value");
    coinaddr = jstr(argjson,"coinaddr");
    if ( (symbol= jstr(argjson,"coin")) != 0 && coinaddr != 0 )
    {
        //char str[65]; printf("uitem %s %s %s/v%d %.8f ht.%d\n",coin,coinaddr,bits256_str(str,txid),vout,dstr(value),height);
        LP_address_utxoadd(LP_coinfind(symbol),coinaddr,txid,vout,value,height,-1);
    }
    return(clonestr("{\"result\":\"success\"}"));
}

void LP_postutxos(char *symbol,char *coinaddr)
{
    bits256 zero; struct iguana_info *coin; cJSON *array,*reqjson = cJSON_CreateObject();
    if ( (coin= LP_coinfind(symbol)) != 0 && (array= LP_address_utxos(coin,coinaddr,1)) != 0 )
    {
        //printf("LP_postutxos pubsock.%d %s %s\n",pubsock,symbol,coin->smartaddr);
        if ( cJSON_GetArraySize(array) == 0 )
            free_json(array);
        else
        {
            memset(zero.bytes,0,sizeof(zero));
            jaddstr(reqjson,"method","postutxos");
            jaddstr(reqjson,"coin",symbol);
            jaddstr(reqjson,"coinaddr",coinaddr);
            jadd(reqjson,"utxos",array);
            //printf("post (%s) -> %d\n",msg,LP_mypubsock);
            LP_reserved_msg(symbol,symbol,zero,jprint(reqjson,1));
        }
    }
}

char *LP_postutxos_recv(cJSON *argjson)
{
    int32_t n; char *symbol,*coinaddr; struct LP_address *ap; struct iguana_info *coin; cJSON *array;
    //printf("posted.(%s)\n",jprint(argjson,0));
    if ( (coinaddr= jstr(argjson,"coinaddr")) != 0 && (symbol= jstr(argjson,"coin")) != 0 && (coin= LP_coinfind(symbol)) != 0 ) // addsig
    {
        if ( coin->electrum == 0 || (ap= LP_addressfind(coin,coinaddr)) != 0 )
        {
            if ( (array= jarray(&n,argjson,"utxos")) != 0 )
                LP_unspents_array(coin,coinaddr,array);
        }
        else if ( (array= electrum_address_listunspent(symbol,coin->electrum,&array,coinaddr,1)) != 0 )
            free_json(array);
    }
    return(clonestr("{\"result\":\"success\"}"));
}

char *LP_pricepings(void *ctx,char *myipaddr,int32_t pubsock,char *base,char *rel,double price)
{
    bits256 zero; cJSON *reqjson = cJSON_CreateObject();
    // LP_addsig
    memset(zero.bytes,0,sizeof(zero));
    jaddbits256(reqjson,"pubkey",G.LP_mypub25519);
    jaddstr(reqjson,"base",base);
    jaddstr(reqjson,"rel",rel);
    jaddnum(reqjson,"price",price);
    jaddstr(reqjson,"method","postprice");
    LP_reserved_msg(base,rel,zero,jprint(reqjson,1));
    return(clonestr("{\"result\":\"success\"}"));
}

char *LP_postprice_recv(cJSON *argjson)
{
    bits256 pubkey; double price; char *base,*rel;
    //printf("PRICE POSTED.(%s)\n",jprint(argjson,0));
    if ( (base= jstr(argjson,"base")) != 0 && (rel= jstr(argjson,"rel")) != 0 && (price= jdouble(argjson,"price")) > SMALLVAL )
    {
        pubkey = jbits256(argjson,"pubkey");
        if ( bits256_nonz(pubkey) != 0 )
        {
            LP_pricefeedupdate(pubkey,base,rel,price);
            return(clonestr("{\"result\":\"success\"}"));
        }
    }
    return(clonestr("{\"error\":\"missing fields in posted price\"}"));
}

void LP_notify_pubkeys(void *ctx,int32_t pubsock)
{
    bits256 zero; char secpstr[67]; cJSON *reqjson = cJSON_CreateObject();
    // LP_addsig
    memset(zero.bytes,0,sizeof(zero));
    jaddstr(reqjson,"method","notify");
    jaddstr(reqjson,"rmd160",G.LP_myrmd160str);
    jaddbits256(reqjson,"pub",G.LP_mypub25519);
    init_hexbytes_noT(secpstr,G.LP_pubsecp,33);
    jaddstr(reqjson,"pubsecp",secpstr);
    LP_reserved_msg("","",zero,jprint(reqjson,1));
}

char *LP_notify_recv(cJSON *argjson)
{
    char *rmd160str,*secpstr; bits256 pub; struct LP_pubkeyinfo *pubp;
    pub = jbits256(argjson,"pub");
    // LP_checksig
    if ( bits256_nonz(pub) != 0 && (rmd160str= jstr(argjson,"rmd160")) != 0 && strlen(rmd160str) == 40 )
    {
        if ( (pubp= LP_pubkeyadd(pub)) != 0 )
        {
            decode_hex(pubp->rmd160,20,rmd160str);
            if ( (secpstr= jstr(argjson,"pubsecp")) != 0 )
            {
                decode_hex(pubp->pubsecp,sizeof(pubp->pubsecp),secpstr);
                //printf("got pubkey.(%s)\n",secpstr);
            }
        }
        //printf("NOTIFIED pub %s rmd160 %s\n",bits256_str(str,pub),rmd160str);
    }
    return(clonestr("{\"result\":\"success\",\"notify\":\"received\"}"));
}

void LP_listunspent_query(char *symbol,char *coinaddr)
{
    bits256 zero; cJSON *reqjson = cJSON_CreateObject();
    memset(zero.bytes,0,sizeof(zero));
    jaddstr(reqjson,"method","addr_unspents");
    jaddstr(reqjson,"coin",symbol);
    jaddstr(reqjson,"address",coinaddr);
    LP_reserved_msg("","",zero,jprint(reqjson,1));
}

void LP_query(void *ctx,char *myipaddr,int32_t mypubsock,char *method,struct LP_quoteinfo *qp)
{
    cJSON *reqjson; bits256 zero; char *msg; int32_t flag = 0; struct LP_utxoinfo *utxo;
    if ( strcmp(method,"request") == 0 )
    {
        if ( (utxo= LP_utxofind(0,qp->desttxid,qp->destvout)) != 0 && LP_ismine(utxo) > 0 && LP_isavailable(utxo) > 0 )
            LP_unavailableset(utxo,qp->srchash);
        else
        {
            printf("couldnt find my txid to make request\n");
            return;
        }
    }
    reqjson = LP_quotejson(qp);
    if ( bits256_nonz(qp->desthash) != 0 )
        flag = 1;
    jaddbits256(reqjson,"pubkey",qp->srchash);
    jaddstr(reqjson,"method",method);
    msg = jprint(reqjson,1);
    // LP_addsig
    printf("QUERY.(%s)\n",msg);
    memset(&zero,0,sizeof(zero));
    portable_mutex_lock(&LP_reservedmutex);
    if ( num_Reserved_msgs < sizeof(Reserved_msgs)/sizeof(*Reserved_msgs) )
        Reserved_msgs[num_Reserved_msgs++] = msg;
    else
    {
        //if ( 1 && strcmp(method,"request") == 0 )
        LP_broadcast_message(LP_mypubsock,qp->srccoin,qp->destcoin,zero,msg);
        //else LP_broadcast_message(LP_mypubsock,qp->srccoin,qp->destcoin,qp->srchash,msg);
    }
    portable_mutex_unlock(&LP_reservedmutex);
}
