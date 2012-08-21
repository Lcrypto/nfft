/*
 * Copyright (c) 2002, 2012 Jens Keiner, Stefan Kunis, Daniel Potts
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

/* $Id: util.c 3483 2010-04-23 19:02:34Z keiner $ */

#include "infft.h"

#if defined(NFFT_LDOUBLE)
  #if LDBL_MANT_DIG > 64
    /* long double 128 bit wide */
    static const R P1[] =
    {
        K(1.00715709113717408460589579223209941204261347125985390244049122),
        K(0.244951997023176876020320575838917179801959212259109588711443322),
        K(0.007157677421552158878119583351756319398653762265084335703499437),
        K(0.000088638803372684623765617528646130117285577351082745565226135),
        K(5.86290506716174293160891590037213629006472953235560773267e-7),
        K(2.345957469650316879673588315314269880856753475545514611e-9),
        K(6.128655873200031753345017339110506400750029700912053e-12),
        K(1.0986892011606471142197070904902679807307454250063e-14),
        K(1.3971139352985872209413265761909675621623302845e-17),
        K(1.2871864763402201040551492995178709443354555e-20),
        K(8.691006015934819586402366491436791599814e-24),
        K(4.309390047550403478410330783891146603e-27),
        K(1.555612957227921944637472907134242e-30),
        K(3.9925388022711127965090640127e-34),
        K(6.9368410036374068977799409e-38),
        K(7.357454531748581565018e-42),
        K(3.62204101214442072e-46),
    };
    static const R Q1[] =
    {
        K(1.000007070784273109528051454385187718969110645914059499666326971665),
        K(-0.005313926453449002844879821442805584438144938596471790433918964154),
        K(7.070788418542362565462155969863941136682888242514039780835548e-6),
        K(-6.26540971437695808708857953483635930152829451235344028056e-9),
        K(4.145433773149898726789014352388133793941127788810021957e-12),
        K(-2.175603200497645955857316018087216481968512641548262e-15),
        K(9.38635995876117135738708302122439402494096616172e-19),
        K(-3.40237206590514988600833435986797066144532635e-22),
        K(1.04920116497922307284730776882365699575587e-25),
        K(-2.7669765371483918815184949580784634161e-29),
        K(6.231564149372584873279011515844227e-33),
        K(-1.188050982913851622946881814029e-36),
        K(1.88293159106694034780937099e-40),
        K(-2.4019827304263718078055e-44),
        K(2.329008315410200325e-48),
        K(-1.5351885198203e-52),
        K(5.185989964e-57),
    };
    static const R P2[] =
    {
      K( 131.0667020533290798466779416062094395070734202211890460026693857),
      K(-245.6303545941878773983069709370348347580041325639555236454969282),
      K( 204.5101554148308448423458719550952945685069457114308003995567211),
      K(-150.5327320302921153754033870344894598759293143264072490756636477),
      K( 98.7912567249134681293252237328232822188103014960829409778733265),
      K(-57.7019241687318590480852440271548151666166507997357208465833492),
      K( 30.1081598594932287475081756049550869990287890532877175670317803),
      K(-14.0088712491952531569334147267018927351724433165829632848926311),
      K( 5.8111909743388254524987383393719023598144356156326152933831237),
      K(-2.1414136626793476610653760169160704073350780611789294609123711),
      K( 0.698259559703142895902111022926235616592885506170181809796723),
      K(-0.2001189347807367129846000381714457873845590394096920427488356),
      K( 0.049988621846192684047867347253361546337456980171441178625268),
      K(-0.0107553807128048320349112565182564637746370430259714241685807),
      K( 0.0019628478791093008488874963820323167136472914702684286586238),
      K(-0.0002975064821201439959012839810796951490827490578144599267114),
      K( 0.0000363948822068787975159479880539390140326476204059270500102),
      K(-3.4500354506312803346122587812035952285744626406680126004e-6),
      K( 2.389194163273735466736165111640837640682648262451843124e-7),
      K(-1.10522228579658847710719851107420853059835206797979003e-8),
      K( 2.998585735858447897961231663140456464729911623332484e-10),
      K(-3.9833617558882606404645795329229040634928477778665e-12),
      K( 1.92588438006565847602499920031655017544553574078e-14),
      K(-1.53988141966712757209882685942469734803820881e-17)
    };
    static const R Q2[] =
    {
      K(328.4489907350967829456654595549647170421869780143590268003242948),
      K(-615.575319575198219705670766725559477112082560070865403723416427),
      K(512.563661235514015228169012911053008163272899480360140632585558),
      K(-377.3415411557897134242242685951784724031156711312221525972796826),
      K(247.6887960380354072297631204916926385047383091206925653242453925),
      K(-144.7079367047916629699737294050207521413589574809430211064537016),
      K(75.5300501439366128901130531145623549019791219037305096822646727),
      K(-35.1562871183824787711155827433330019401972267325168728838283815),
      K(14.5900995053625141158588399870084373879991873744753473985888771),
      K(-5.3793167684795558315535956122846115342243771791771104555817078),
      K(1.7551860916884001665224751521627837644140913167505738803593836),
      K(-0.5034275191915128601243198185547065251560284487106213345738464),
      K(0.1258761177781805904401665597080296597844219207555140252667602),
      K(-0.0271165711067811527517031577204913417520417117202287079370524),
      K(0.0049566968097755025323253093106955508273120444565853663949773),
      K(-0.0007529042621964042565348473109633952383792675486716081916002),
      K(0.0000923845012025941529926888028246088283227759969619552077927),
      K(-8.7971061636613823628316954758674029940675792754742342069e-6),
      K(6.136252042100256280345626781919643641626278378156829218e-7),
      K(-2.87514497545628209951433938237642236070094379906239019e-8),
      K(7.99982739755782677965258184972152681716223485333226e-10),
      K(-1.12250050360331111260828032356887810736651792918878e-11),
      K(6.221827386743849038481605158298006999161167883e-14),
      K(-8.0950402455773560717067392049638116407074069e-17)
    };
  #elif LDBL_MANT_DIG == 64
    /* long double 96 bit wide */
    static const R P1[] =
    {
        K(1.00696388290874250231638626673686646317801154370159972703168538),
        K(0.243352848727738955738908687369450214577920342918851509272408866),
        K(0.006964401160721188186398281247079919082283450941469460164590432),
        K(0.000083047334117897959145500056901191736030823531931380263686302),
        K(5.18256420384764810882467760619532575731801821889985626099e-7),
        K(1.90790611016475818883461118145629029943434891680660527e-9),
        K(4.44170587990105074420754325358582895345307949815573e-12),
        K(6.805150196466153819995090798791966304827558189423e-15),
        K(6.985104315031938858779570788468047860794936128e-18),
        K(4.785507068734939741097928056648844894386614e-21),
        K(2.117077490896605677726199140622837572025e-24),
        K(5.52919580174986488729896702518475621e-28),
        K(6.5666588969169003434516942087381e-32),
    };
    static const R Q1[] =
    {
        K(1.000011863675914860400478598182318948452642744176732473923183795358),
        K(-0.006896324225185339751945118908659032102601115341265224641280055928),
        K(0.00001186368725103095674191039208189536570056102297912907572890034),
        K(-1.3496696876875206170114218872940978330152830864048922600366e-8),
        K(1.1336174449932022007831556161183407392648142591512923821e-11),
        K(-7.422841655569707018644701823047396523966712871897812e-15),
        K(3.893669273036094904159100761627937282479044304922e-18),
        K(-1.651720545895290413869725701665259282787265642e-21),
        K(5.6323805635535562808481781647661895955232e-25),
        K(-1.50728066570923164846664348266414336373e-28),
        K(3.006044492319661074666639642233229e-32),
        K(-4.010148023149017379419706572721e-36),
        K(2.70282874465984817539266054e-40),
    };
    static const R P2[] =
    {
      K( 1.30090423521760256476093919023146864017751590623897710895862681),
      K(-1.981041925270972574120174940817336830170017871902975653312750388),
      K( 0.956892580228917795561363651877698243164566364537052353014543669),
      K(-0.295476285312266394050596510402082979039773201845265239542019439),
      K( 0.056978837924988815165935230495950981635872574537538384147033652),
      K(-0.006299149197554616295736173514236214970859775932020376086036399),
      K( 0.000353716966863384475462973243411450895641022763240331882363443),
      K(-8.707624424632528381900923003415938761710942641810978203625e-6),
      K( 7.283705999222063845686558855093093825421931939071318202e-8),
      K(-9.7967727386492889920273780071218382357131320542055799e-11)
    };
    static const R Q2[] =
    {
      K( 3.257608431020108786259398271424889402309379351594793640349460063),
      K(-4.96363276525502538609792324882976732173260916421950408693842731),
      K( 2.400495835659089927333294199555080092801133193497330702140754591),
      K(-0.742868968166381852162379299256973953894545292197384361382965225),
      K( 0.143801810439830068463911726822151703498931831272162081681231077),
      K(-0.016019224718850575023820322478614758671031103220377245057110662),
      K( 0.000914623505897601721718970098041677534130323750396506936364887),
      K(-0.000023411644633126949191317085153966622167096685843127825287574),
      K( 2.17705048674331703171406080664526952334380771487046428339e-7),
      K(-4.47580289731041130181939560179689655281441839562189718e-10)
    };
  #else
    #error Unsupported size of long double
  #endif
#elif defined(NFFT_SINGLE)
  /* float */
  static const R P1[] =
  {
      K(1.006634511033311726164163027592274220828216885723379609007274761),
      K(0.240606487720090757394176928596156553834296465200311569457994763),
      K(0.006634921274522227156198202198389031672287220144321235665461021),
      K(0.000073749622820821337100502174723273851941734199062726870961819),
      K(4.10243517822171814488230564074819973544765129449450710122e-7),
      K(1.262110026222369902633819303536802438120823461060572684e-9),
      K(2.218532296437410634454463125960648541194468552527652e-12),
      K(2.141504045536019682125761418851096299425878119158e-15),
      K(9.19584570350722374435337612379408707845677156e-19),
  };
  static const R Q1[] =
  {
      K(1.000022624782705275228334312456728477812835742762369533496905023937),
      K(-0.009614857078745003693609489751018087358244444264456521971379273084),
      K(0.000022624818652773047747424411495054891627754515915461183178099877),
      K(-3.4080521639954323706277061786236961377055349443081338572762e-8),
      K(3.5947512112800645225066705862453058797853924958888263259e-11),
      K(-2.7149805873212658218594464017972758572144265290831215e-14),
      K(1.4293388301569282795540255590126107486209476445158e-17),
      K(-4.771887851505849942903948600229238419570937509e-21),
      K(7.68298982666756594543081799488936861257839e-25),
  };
  static const R P2[] =
  {
    K( 0.400758393969643840397216812932361963736749407866811083462461),
    K(-0.0312216150704950438088565774064329777860642477326179964345542),
    K( 0.0001215451718646727844117193541329442989170354233955281424116)
  };
  static const R Q2[] =
  {
    K( 1.00043733569136882353241680221279480297575523819814430369272934),
    K(-0.0822433017391967535749382764476705160129315137731445852657631),
    K( 0.00043733569136882353241680221279480297575523819814430369272934)
  };
#else
  /* double */
  static const R P1[] =
  {
      K(1.006897990143384859657820271920512961153421109156614230747188622),
      K(0.242805341483041870658834102275462978674549112393424086979586278),
      K(0.006898486035482686938510112687043665965094733332210445239567379),
      K(0.000081165067173822070066416843139523709162208390998449005642346),
      K(4.95896034564955471201271060753697747487292805350402943964e-7),
      K(1.769262324717844587819564151110983803173733141412266849e-9),
      K(3.936742942676484111899247866083681245613312522754135e-12),
      K(5.65030097981781148787580946077568408874044779529e-15),
      K(5.267856044117588097078633338366456262960465052e-18),
      K(3.111192981528832405775039015470693622536939e-21),
      K(1.071238669051606108411504195862449904664e-24),
      K(1.66685455020362122704904175079692613e-28),
  };
  static const R Q1[] =
  {
      K(1.000013770640886533569435896302721489503868900260448440877422679934),
      K(-0.007438195256024963574139196893944950727405523418354136393367554385),
      K(0.000013770655915064256304772604385297068669909609091264440116789601),
      K(-1.6794623118559896441239590667288215019925076196457659206142e-8),
      K(1.50285363491992136130760477001818578470292828225498818e-11),
      K(-1.0383232801211938342796582949062551517465351830706356e-14),
      K(5.66233115275307483428203764087829782195312564006e-18),
      K(-2.44062252162491829675666639093292109472275754e-21),
      K(8.15441695513966815222186223740016719597617e-25),
      K(-2.01117218503954384746303760121365911698e-28),
      K(3.2919820158429806312377323449729691e-32),
      K(-2.70343047912331415988664032397e-36),
  };
  static const R P2[] =
  {
    K( 0.4305671332839579065931339658100499864903788418438938270811),
    K(-0.2897224581554843285637983312103876003389911968369470222427),
    K( 0.0299419330186508349765969995362253891383950029259740306077),
    K(-0.0010756807437990349677633120240742396555192749710627626584),
    K( 0.0000116485185631252780743187413946316104574410146692335443),
    K(-1.89995137955806752293614125586568854200245376235433e-08)
  };
  static const R Q2[] =
  {
    K(1.0762291019783101702628805159947862543863829764738274558421),
    K(-0.7279167074883770739509279847502106137135422309409220238564),
    K(0.0762629142282649564822465976300194596092279190843683614797),
    K(-0.0028345107938479082322784040228834113914746923069059932628),
    K(0.0000338122499547862193660816352332052228449426105409056376),
    K(-8.28850093512263912295888947693700479250899073022595e-08)
  };
#endif

static const INT N1 = sizeof(P1)/sizeof(P1[0]);
static const INT M1 = sizeof(Q1)/sizeof(Q1[0]);
static const INT N2 = sizeof(P2)/sizeof(P2[0]);
static const INT M2 = sizeof(Q2)/sizeof(Q2[0]);

static inline R evaluate_chebyshev(const INT n, const R *c, const R x)
{
  R a = c[n-2], b = c[n-1], t;
  INT j;
  
  A(n >= 2);
  
  for (j = n - 2; j > 0; j--)
  {
    t = c[j-1] - b;
    b = a + K(2.0) * x * b;
    a = t;
  }
  return a + x * b;
}

R Y(bessel_i0)(R x)
{
  if (x < 0)
  {
    /* even function */
    x = -x;
  }

  if (x == K(0.0))
    return K(1.0);

  if (x <= K(15.0))
  {
    /* x in (0, 15] */
    const R y = x * x;
    return evaluate_chebyshev(N1, P1, y) / evaluate_chebyshev(M1, Q1, y);
  }
  else
  {
    /* x in (15, \infty) */
    const R y = (K(30.0) - x) / x;
    return (EXP(x) / SQRT(x)) * (evaluate_chebyshev(N2, P2, y) /
      evaluate_chebyshev(M2, Q2, y));
  }
}
