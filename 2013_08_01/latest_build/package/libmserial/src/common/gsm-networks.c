/*

  $Id: gsm-networks.c,v 1.47 2007/11/07 18:28:18 pkot Exp $

  G N O K I I

  A Linux/Unix toolset and driver for the mobile phones.

  This file is part of gnokii.

  Gnokii is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  Gnokii is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with gnokii; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

  Copyright (C) 1999-2000 Hugh Blemings & Pavel Jan�k ml.
  Copyright (C) 2002-2003 Pawel Kot, Ladis Michl
  Copyright (C) 2002      Feico de Boer

  This file implements GSM networks searching.

*/

#include <string.h>

#include "config.h"
#include "compat.h"
#include "misc.h"
#include "gnokii.h"

static gn_country countries[] = {
	{"202", "Greece"},
	{"204", "Netherlands"},
	{"206", "Belgium"},
	{"208", "France"},
	{"213", "Andorra"},
	{"214", "Spain"},
	{"216", "Hungary"},
	{"218", "Bosnia Herzegovina"},
	{"219", "Croatia"},
	{"220", "Serbia and Montenegro"},
	{"222", "Italy"},
	{"226", "Romania"},
	{"228", "Switzerland"},
	{"230", "Czech Republic"},
	{"231", "Slovak Republic"},
	{"232", "Austria"},
	{"234", "Guernsey"},
	{"234", "Isle of Man"},
	{"234", "Jersey"},
	{"234", "United Kingdom"},
	{"238", "Denmark"},
	{"240", "Sweden"},
	{"242", "Norway"},
	{"244", "Finland"},
	{"246", "Lithuania"},
	{"247", "Latvia"},
	{"248", "Estonia"},
	{"250", "Russia"},
	{"255", "Ukraine"},
	{"257", "Belarus"},
	{"259", "Moldova"},
	{"260", "Poland"},
	{"262", "Germany"},
	{"266", "Gibraltar"},
	{"268", "Portugal"},
	{"270", "Luxembourg"},
	{"272", "Ireland"},
	{"274", "Iceland"},
	{"276", "Albania"},
	{"278", "Malta"},
	{"280", "Cyprus"},
	{"282", "Georgia"},
	{"283", "Armenia"},
	{"283", "Nagorno Karabakh Region"},
	{"284", "Bulgaria"},
	{"286", "Turkey"},
	{"288", "Faroe Islands"},
	{"290", "Greenland"},
	{"293", "Slovenia"},
	{"294", "Macedonia"},
	{"295", "Liechtenstein"},
	{"302", "Canada"},
	{"310", "USA"},
	{"311", "USA"},
	{"334", "Mexico"},
	{"338", "Jamaica"},
	{"340", "French West Indies"},
	{"342", "Barbados"},
	{"344", "Antigua & Barbuda"},
	{"346", "Cayman Islands"},
	{"350", "Bermuda"},
	{"352", "Grenada"},
	{"362", "Netherlands Antilles"},
	{"363", "Aruba"},
	{"364", "Bahamas"},
	{"368", "Cuba"},
	{"370", "Dominican Republic"},
	{"374", "Trinidad and Tobago"},
	{"400", "Azerbaijan"},
	{"401", "Kazakhstan"},
	{"402", "Bhutan"},
	{"404", "India"},
	{"410", "Pakistan"},
	{"412", "Afghanistan"},
	{"413", "Sri Lanka"},
	{"414", "Myanmar"},
	{"415", "Lebanon"},
	{"416", "Jordan"},
	{"417", "Syria"},
	{"419", "Kuwait"},
	{"420", "Saudi Arabia"},
	{"421", "Yemen"},
	{"422", "Oman"},
	{"424", "United Arab Emirates"},
	{"425", "Israel"},
	{"425", "Palestinian Authority"},
	{"426", "Bahrain"},
	{"427", "Qatar"},
	{"428", "Mongolia"},
	{"429", "Nepal"},
	{"432", "Iran"},
	{"434", "Uzbekistan"},
	{"436", "Tajikistan"},
	{"437", "Kyrgyz Republic"},
	{"438", "Turkmenistan"},
	{"440", "Japan"},
	{"450", "South Korea"},
	{"452", "Vietnam"},
	{"454", "Hong Kong"},
	{"455", "Macau"},
	{"456", "Cambodia"},
	{"457", "Lao"},
	{"460", "China"},
	{"466", "Taiwan"},
	{"467", "North Korea"},
	{"470", "Bangladesh"},
	{"472", "Maldives"},
	{"502", "Malaysia"},
	{"505", "Australia"},
	{"510", "Indonesia"},
	{"514", "East Timor"},
	{"515", "Philippines"},
	{"520", "Thailand"},
	{"525", "Singapore"},
	{"528", "Brunei Darussalam"},
	{"530", "New Zealand"},
	{"537", "Papua New Guinea"},
	{"539", "Tonga"},
	{"541", "Vanuatu"},
	{"542", "Fiji"},
	{"545", "Kiribati"},
	{"546", "New Caledonia"},
	{"547", "French Polynesia"},
	{"550", "Micronesia"},
	{"602", "Egypt"},
	{"603", "Algeria"},
	{"604", "Morocco"},
	{"605", "Tunisia"},
	{"607", "Gambia"},
	{"608", "Senegal"},
	{"609", "Mauritania"},
	{"610", "Mali"},
	{"611", "Guinea"},
	{"612", "Cote d'Ivoire"},
	{"613", "Burkina Faso"},
	{"614", "Niger"},
	{"615", "Togo"},
	{"616", "Benin"},
	{"617", "Mauritius"},
	{"618", "Liberia"},
	{"619", "Sierra Leone"},
	{"620", "Ghana"},
	{"621", "Nigeria"},
	{"622", "Chad"},
	{"624", "Cameroon"},
	{"625", "Cape Verde"},
	{"626", "Sao Tome and Principe"},
	{"627", "Equatorial Guinea"},
	{"628", "Gabon"},
	{"629", "Congo"},
	{"630", "Congo"},
	{"631", "Angola"},
	{"633", "Seychelles"},
	{"634", "Sudan"},
	{"635", "Rwanda"},
	{"636", "Ethiopia"},
	{"637", "Somalia"},
	{"638", "Djibouti"},
	{"639", "Kenya"},
	{"640", "Tanzania"},
	{"641", "Uganda"},
	{"642", "Burundi"},
	{"643", "Mozambique"},
	{"645", "Zambia"},
	{"646", "Madagascar"},
	{"647", "Reunion (La)"},
	{"648", "Zimbabwe"},
	{"649", "Namibia"},
	{"650", "Malawi"},
	{"651", "Lesotho"},
	{"652", "Botswana"},
	{"653", "Swaziland"},
	{"654", "Comoros"},
	{"655", "South Africa"},
	{"702", "Belize"},
	{"706", "El Salvador"},
	{"710", "Nicaragua"},
	{"712", "Costa Rica"},
	{"714", "Panama"},
	{"716", "Peru"},
	{"722", "Argentina"},
	{"724", "Brazil"},
	{"730", "Chile"},
	{"732", "Colombia"},
	{"734", "Venezuela"},
	{"736", "Bolivia"},
	{"740", "Ecuador"},
	{"744", "Paraguay"},
	{"746", "Suriname"},

	{ NULL, NULL }
};

static gn_network networks[] = {
	{"202 01", "Cosmote"},
	{"202 05", "VODAFONE-PANAFON"},
	{"202 09", "Q-TELECOM"},
	{"202 10", "TELESTET"},
	{"204 04", "Vodafone"},
	{"204 08", "KPN Mobile The Netherlands BV"},
	{"204 12", "Telfort B.V."},
	{"204 16", "T-Mobile NL"},
	{"204 20", "Orange Nederland N.V."},
	{"206 01", "PROXIMUS"},
	{"206 10", "Mobistar"},
	{"206 20", "BASE"},
	{"208 01", "Orange F"},
	{"208 10", "SFR"},
	{"208 20", "Bouygues Telecom"},
	{"213 03", "MOBILAND"},
	{"214 01", "vodafone"},
	{"214 03", "AMENA"},
	{"214 04", "Xfera"},
	{"214 07", "MOVISTAR"},
	{"216 01", "PANNON GSM"},
	{"216 30", "WESTEL"},
	{"216 70", "Vodafone"},
	{"218 03", "ERONET"},
	{"218 05", "Mobilna Srpske"},
	{"218 90", "GSMBIH"},
	{"219 01", "HTmobile"},
	{"219 10", "VIPnet"},
	{"220 01", "MOBTEL"},
	{"220 02", "YU 02"},
	{"220 03", "Telekom Srbija"},
	{"220 04", "MONET"},
	{"222 01", "Telecom Italia Mobile"},
	{"222 10", "vodafone"},
	{"222 88", "Wind Telecomunicazioni SpA"},
	{"222 98", "Blu SpA"}, /* defunct year 2002 */
	{"222 99", "H3G"},
	{"226 01", "CONNEX GSM"},
	{"226 03", "Cosmorom S.A."},
	{"226 10", "ORANGE"},
	{"228 01", "SWISS GSM"},
	{"228 02", "sunrise"},
	{"228 03", "ORANGE"},
	{"230 01", "T-Mobile CZ"},
	{"230 02", "EUROTEL PRAHA"},
	{"230 03", "Oskar Mobil"},
	{"231 01", "Orange SK"},
	{"231 02", "EUROTEL GSM"},
	{"232 01", "A1"},
	{"232 01", "A1-3G"},
	{"232 03", "T-Mobile A"},
	{"232 05", "one"},
	{"232 07", "Tele.ring"},
	{"232 10", "3 AT"},
	{"234 10", "O2"},
	{"234 15", "Vodafone"},
	{"234 20", "3 UK"},
	{"234 30", "T-Mobile UK"},
	{"234 33", "ORANGE"},
	{"234 50", "JT GSM"},
	{"234 55", "Cable & Wireless Guernsey"},
	{"234 58", "Pronto GSM"},
	{"238 01", "TDC Mobil"},
	{"238 02", "SONOFON"},
	{"238 20", "TELIA DK"},
	{"238 30", "Orange"},
	{"240 01", "TELIA MOBILE"},
	{"240 02", "3"},
	{"240 07", "COMVIQ"},
	{"240 08", "Vodafone"},
	{"242 01", "TELENOR"},
	{"242 02", "NetCom"},
	{"244 03", "TELIA"},
	{"244 05", "Radiolinja Origo Oy"},
	{"244 09", "Finnet"},
	{"244 12", "Suomen 2G Oy"},
	{"244 14", "Alands Mobiltelefon AB"},
	{"244 91", "Sonera"},
	{"246 01", "OMNITEL"},
	{"246 02", "BITE GSM"},
	{"246 03", "TELE2"},
	{"247 01", "LMT GSM"},
	{"247 02", "TELE2"},
	{"248 01", "EMT GSM"},
	{"248 02", "Radiolinja Eesti"},
	{"248 03", "TELE2"},
	{"250 01", "Mobile Telesystems"},
	{"250 01", "SANTEL"},
	{"250 01", "Tambov RUS"},
	{"250 02", "MegaFon"},
	{"250 02", "MegaFon Moscow"},
	{"250 03", "NCC"},
	{"250 04", "SIBCHALLENGE LTD"},
	{"250 05", "Mobile Comms Systems"},
	{"250 05", "Sayantelecom LLC"},
	{"250 05", "SCS-900"},
	{"250 05", "Tomsk Cellular Communication"},
	{"250 05", "Yeniseitelecom"},
	{"250 07", "BM Telecom"},
	{"250 07", "SMARTS"},
	{"250 10", "DTC"},
	{"250 11", "Orensot"},
	{"250 12", "BaykalWestCom"},
	{"250 12", "Far Eastern Cellular Systems"},
	{"250 12", "Sakhalin GSM"},
	{"250 12", "Sibintertelecom"},
	{"250 12", "ULAN-UDE CELLULAR NETWORK"},
	{"250 13", "Kuban-GSM"},
	{"250 15", "SMARTS-Ufa"},
	{"250 16", "NTC"},
	{"250 17", "Ermak RMS"},
	{"250 19", "BASHCELL"},
	{"250 19", "Dal Telecom International"},
	{"250 19", "INDIGO"},
	{"250 20", "Cellular Communcations of Udmurtia"},
	{"250 20", "MOTIV"},
	{"250 20", "TELE2"},
	{"250 20", "TELE2-OMSK"},
	{"250 28", "Extel"},
	{"250 39", "JSC Uralsvyazinform"},
	{"250 39", "South Ural Cellular Telephone"},
	{"250 39", "Uraltel"},
	{"250 44", "North Caucasian GSM"},
	{"250 92", "Primtelefone"},
	{"250 99", "Bee Line GSM"},
	{"250 99", "BeeLine GSM"},
	{"255 01", "UMC"},
	{"255 02", "WellCOM"},
	{"255 03", "KYIVSTAR"},
	{"255 05", "Golden Telecom GSM"},
	{"255 06", "life:)"},
	{"257 01", "VELCOM"},
	{"257 02", "MTS"},
	{"259 01", "VoXtel S.A."},
	{"259 02", "Moldcell"},
	{"260 01", "PLUS GSM"},
	{"260 02", "Era"},
	{"260 03", "IDEA"},
	{"262 01", "D1"},
	{"262 02", "Vodafone"},
	{"262 03", "E-Plus"},
	{"262 07", "O2 (Germany) GmbH & Co. OHG"},
	{"262 13", "Mobilcom Multimedia GMBH"},
	{"266 01", "Gibtel GSM"},
	{"268 01", "vodafone"},
	{"268 03", "OPTIMUS"},
	{"268 06", "P TMN"},
	{"270 01", "LUXGSM"},
	{"270 77", "TANGO"},
	{"272 01", "Vodafone"},
	{"272 02", "O2 Communications (Ireland) Ltd"},
	{"272 03", "METEOR"},
	{"274 01", "LANDSSIMINN"},
	{"274 02", "TAL"},
	{"274 03", "Islandssimi GSM ehf"},
	{"274 04", "Viking Wireless"},
	{"276 01", "A M C MOBIL"},
	{"276 02", "vodafone"},
	{"278 01", "Vodafone Malta"},
	{"278 21", "go mobile"},
	{"280 01", "CYTA"},
	{"282 01", "Geocell Limited"},
	{"282 02", "Magti GSM"},
	{"283 01", "ARMGSM"},
	{"283 04", "NKRGSM"},
	{"284 01", "M-TEL GSM BG"},
	{"284 05", "GLOBUL"},
	{"286 01", "Turkcell Iletisim Hizmetleri A.S."},
	{"286 02", "TELSIM GSM"},
	{"286 03", "ARIA"},
	{"286 04", "AYCELL"},
	{"288 01", "Faroese Telecom GSM"},
	{"288 02", "KALL-GSM"},
	{"290 01", "Tele Greenland"},
	{"293 40", "SI.MOBIL"},
	{"293 41", "MOBITEL"},
	{"293 70", "VEGA"},
	{"294 01", "MOBIMAK"},
	{"294 02", "MTS A.D"},
	{"295 01", "Telecom FL AG"},
	{"295 02", "Orange FL"},
	{"295 05", "FL1"},
	{"295 77", "Tele 2 AG"},
	{"302 37", "MICROCELL"},
	{"302 72", "Rogers Wireless"},
	{"310 00", "Mid-Tex Cellular"},
	{"310 03", "Centennial Communications"},
	{"310 03", "Indigo Wireless"},
	{"310 04", "Concho Cellular Telephone Co."},
	{"310 07", "Highland"},
	{"310 08", "Corr Wireless Communications"},
	{"310 10", "Plateau Wireless"},
	{"310 15", "Cingular Wireless"},
	{"310 16", "T-Mobile USA"},
	{"310 19", "Dutch Harbor"},
	{"310 26", "Cook Inlet T-Mobile USA"},
	{"310 32", "Smith Bagley"},
	{"310 34", "Westlink Communications"},
	{"310 38", "AT&T Wireless"},
	{"310 45", "North East Colorado Cellular"},
	{"310 46", "TMP Corp"},
	{"310 50", "PSC Wireless"},
	{"310 53", "West Virginia Wireless"},
	{"310 56", "Dobson"},
	{"310 59", "Cellular One"},
	{"310 63", "Choice Wireless L.C."},
	{"310 64", "Airadigm Communications"},
	{"310 68", "NPI Wireless"},
	{"310 69", "Immix Wireless"},
	{"310 74", "TELEMETRIX"},
	{"310 77", "Iowa Wireless Services LP"},
	{"310 79", "PinPoint Wireless"},
	{"310 94", "Poka Lambro Telcomm. Ltd. d/b/a Digital Cellular"},
	{"310 94", "Poka Lambro Telecomm. Ltd. d/b/a Digital Cellular"},
	{"311 11", "High Plains Wireless, L.P"},
	{"334 02", "TELCEL GSM"},
	{"334 03", "MOVISTAR GSM"},
	{"338 05", "Digicel"},
	{"338 18", "Cable and Wireless Jamaica Limited"},
	{"340 01", "Orange"},
	{"340 03", "Saint Martin et Saint Barthelemy Tel Cell SARL"},
	{"340 08", "AMIGO GSM"},
	{"340 20", "Bouygues Telecom Caraibe"},
	{"342 60", "Cable & Wireless (Barbados) Limited"},
	{"344 03", "APUA PCS"},
	{"346 14", "Cable & Wireless (Cayman Islands) Limited"},
	{"350 01", "Telecommunications (Bermuda & West Indies) Ltd"},
	{"350 02", "MOBILITY LTD."},
	{"352 13", "Trans-World Telecom Caribbean (Grenada)"},
	{"362 51", "Telcell N.V."},
	{"362 69", "CT GSM"},
	{"362 91", "UTS Wireless Curacao"},
	{"363 01", "SETAR GSM - GSM 1800"},
	{"363 01", "SETAR GSM - GSM 1900"},
	{"363 01", "SETAR GSM - GSM 900"},
	{"364 39", "The Bahamas Telecommunications Company Ltd"},
	{"368 01", "C_Com"},
	{"370 01", "ORANGE"},
	{"374 12", "TSTT"},
	{"400 01", "AZERCELL GSM"},
	{"400 02", "BAKCELL GSM 2000"},
	{"401 01", "K-MOBILE"},
	{"401 02", "K'CELL"},
	{"402 11", "B-Mobile"},
	{"404 01", "Aircel Digilink India Limited - Haryana"},
	{"404 02", "AirTel - Punjab"},
	{"404 03", "AirTel - Himachal Pradesh Circle"},
	{"404 03", "Hutch - City of Calcutta"},
	{"404 04", "IDEA - Delhi Circle"},
	{"404 05", "HUTCH"},
	{"404 07", "IDEA - Andhra Pradesh Circle"},
	{"404 09", "Reliance Telecom Private Ltd"},
	{"404 10", "AirTel - Delhi"},
	{"404 11", "HUTCH"},
	{"404 12", "ESCOTEL Haryana"},
	{"404 13", "HESL - Andhra Pradesh"},
	{"404 14", "SPICE - Punjab"},
	{"404 15", "Aircel Digilink India Limited - UP East"},
	{"404 19", "Escotel Kerala"},
	{"404 20", "Orange"},
	{"404 21", "BPL - Mobile - Mumbai"},
	{"404 22", "IDEA - Maharashtra Circle"},
	{"404 24", "IDEA - Gujarat Circle"},
	{"404 27", "BPL Mobile - Maharashtra/Goa"},
	{"404 31", "AirTel - Kolkata"},
	{"404 34", "CellOne"},
	{"404 40", "AIRTEL - Chennai"},
	{"404 41", "RPG Cellular"},
	{"404 42", "AIRCEL"},
	{"404 43", "BPL Mobile - Tamil Nadu/Pondicherry"},
	{"404 44", "Spice - Karnataka"},
	{"404 45", "Airtel - Karnataka"},
	{"404 46", "BPL Mobile - Kerala"},
	{"404 49", "Airtel - Andhra Pradesh"},
	{"404 56", "Escotel UP(W)"},
	{"404 60", "Aircel Digilink India Limited - Rajasthan"},
	{"404 68", "Mahanagar Telephone Nigam Ltd - Delhi"},
	{"404 69", "Mahanagar Telephone Nigam Ltd - Mumbai"},
	{"404 70", "Oasis Cellular"},
	{"404 78", "IDEA - Madhya Pradesh"},
	{"404 84", "HESL - Chennai"},
	{"404 86", "HESL - Karnataka"},
	{"404 90", "AirTel - Maharashtra"},
	{"404 92", "AirTel - Mumbai Metro"},
	{"404 93", "AirTel - Madhya Pradesh"},
	{"404 94", "AirTel - Tamilnadu"},
	{"404 95", "AirTel - Kerala"},
	{"404 96", "AirTel - Haryana"},
	{"404 97", "AirTel - Uttar Pradesh (West)"},
	{"404 98", "AirTel - Gujarat"},
	{"410 01", "Mobilink"},
	{"410 03", "Ufone"},
	{"412 01", "AWCC"},
	{"412 20", "TDCA"},
	{"413 02", "DIALOG GSM"},
	{"413 03", "Celltel Infiniti"},
	{"413 07", "Mobitel (Pvt) Limited"},
	{"414 01", "MPT GSM Network"},
	{"415 01", "Cellis"},
	{"415 03", "LibanCell"},
	{"416 01", "Fastlink - Jordan"},
	{"416 77", "MOBILECOM"},
	{"417 01", "SYRIATEL"},
	{"417 02", "94"},
	{"417 09", "MOBILE SYRIA"},
	{"419 02", "MTCNet"},
	{"419 03", "Wataniya Telecom"},
	{"420 01", "AL JAWAL"},
	{"421 01", "Yemen Company for Mobile Telephony"},
	{"421 02", "Spacetel"},
	{"422 02", "OMAN MOBILE"},
	{"424 02", "ETISALAT"},
	{"425 01", "Orange"},
	{"425 02", "Cellcom"},
	{"425 02", "Cellcom Israel Ltd - 3G"},
	{"425 05", "Palestine Telecommunications Co. P.L.C"},
	{"426 01", "BHR MOBILE PLUS"},
	{"426 02", "MTC VODAFONE Bahrain"},
	{"427 01", "QATARNET"},
	{"428 99", "MobiCom"},
	{"429 01", "Nepal Mobile"},
	{"432 11", "TCI"},
	{"432 14", "KFZO"},
	{"432 19", "MTCE"},
	{"434 02", "Uzmacom"},
	{"434 04", "Daewoo Unitel"},
	{"434 05", "Coscom"},
	{"434 07", "Uzdunrobita GSM"},
	{"436 01", "JSC Somoncom"},
	{"436 02", "Indigo Tajikistan"},
	{"436 03", "Mobile Lines of Tajikistan"},
	{"436 05", "Tajik Tel"},
	{"437 01", "BITEL"},
	{"438 01", "BCTI"},
	{"440 10", "NTT DoCoMo, Inc"},
	{"440 20", "J-Phone"},
	{"450 02", "KT ICOM"},
	{"452 01", "MOBIFONE"},
	{"452 02", "Vinaphone"},
	{"454 00", "CSL GSM 900/1800"},
	{"454 03", "3"},
	{"454 04", "Orange-Dual Band"},
	{"454 06", "SmarTone"},
	{"454 10", "New World Mobility"},
	{"454 12", "PEOPLES"},
	{"454 16", "SUNDAY"},
	{"455 00", "SMC Macau"},
	{"455 01", "TELEMOVEL+ GSM900-Macau"},
	{"455 03", "Hutchison Telecom"},
	{"456 01", "MOBITEL"},
	{"456 02", "SAMART"},
	{"456 18", "Cambodia Shinawatra"},
	{"457 01", "LAO TELECOMMUNICATIONS"},
	{"457 02", "ETL Mobile"},
	{"457 03", "LAT MOBILE"},
	{"457 08", "Millicom Lao Co Ltd"},
	{"460 00", "China Mobile"},
	{"460 01", "CU-GSM"},
	{"466 01", "Far EasTone GSM 900/1800"},
	{"466 88", "KG Telecom"},
	{"466 89", "T3G"},
	{"466 92", "LDTA GSM"},
	{"466 93", "MobiTai"},
	{"466 97", "Pacific GSM 1800"},
	{"466 99", "TransAsia Telecommunications"},
	{"467 19", "Sun Net"},
	{"470 01", "GrameenPhone"},
	{"470 02", "AKTEL"},
	{"470 03", "ShebaWorld"},
	{"472 01", "DhiMobile GSM 900"},
	{"502 12", "Maxis Mobile"},
	{"502 12", "MMS & MB"},
	{"502 13", "TMTOUCH"},
	{"502 16", "DiGi"},
	{"502 19", "CELCOM"},
	{"505 01", "Telstra MobileNet"},
	{"505 02", "YES OPTUS"},
	{"505 03", "vodafone"},
	{"505 06", "Hutchison 3G Australia Pty Limited"},
	{"510 01", "IND SATELINDOCEL"},
	{"510 08", "Lippo Telecom"},
	{"510 10", "TELKOMSEL"},
	{"510 11", "Excelcom"},
	{"510 21", "INDOSAT-M3"},
	{"514 02", "Timor Telecom"},
	{"515 01", "ISLACOM"},
	{"515 02", "Globe Telecom"},
	{"515 03", "Smart Gold GSM"},
	{"515 05", "Digitel Mobile/Sun Cellular"},
	{"520 01", "AIS GSM"},
	{"520 15", "ACT Mobile"},
	{"520 18", "DTAC"},
	{"520 23", "GSM 1800"},
	{"520 99", "TA Orange Co"},
	{"525 01", "SingTel ST-GSM 900"},
	{"525 02", "SingTel-G18"},
	{"525 03", "MOBILEONE"},
	{"525 05", "StarHub Pte Ltd"},
	{"528 11", "DSTCom"},
	{"530 01", "vodafone"},
	{"537 01", "Bee Mobile"},
	{"539 01", "U-CALL"},
	{"539 43", "Shoreline Communications"},
	{"541 01", "SMILE"},
	{"542 01", "VODAFONE"},
	{"545 09", "Kiribati Frigate"},
	{"546 01", "Mobilis"},
	{"547 20", "VINI"},
	{"550 01", "FSM Telecommunications Corporation"},
	{"602 01", "ECMS-MobiNil"},
	{"602 02", "Vodafone Egypt"},
	{"603 01", "AMN"},
	{"603 02", "Djezzy"},
	{"604 00", "Meditel"},
	{"604 01", "I A M"},
	{"605 02", "TUNTEL"},
	{"605 03", "TUNISIANA"},
	{"607 01", "Gamcell"},
	{"607 02", "AFRICELL"},
	{"608 01", "ALIZE"},
	{"608 02", "Sentel GSM"},
	{"609 01", "MATTEL"},
	{"609 10", "MAURITEL"},
	{"610 01", "Malitel"},
	{"610 02", "IKATEL"},
	{"611 01", "Mobilis Guinee"},
	{"611 02", "Lagui"},
	{"612 01", "CORA de COMSTAR"},
	{"612 03", "Orange CI"},
	{"612 05", "TELECEL"},
	{"613 02", "Celtel Burkina Faso"},
	{"613 03", "Telecel Faso SA"},
	{"614 02", "Celtel Niger"},
	{"614 03", "TELECEL Niger"},
	{"615 01", "TOGOCEL"},
	{"616 00", "Bell Benin Communication BBCOM"},
	{"616 01", "LIBERCOM"},
	{"616 02", "TELECEL BENIN"},
	{"616 03", "BeninCell"},
	{"617 01", "Cellplus Mobile Comms"},
	{"617 10", "EMTEL"},
	{"618 01", "Lonestar Cell"},
	{"619 01", "Celtel (SL) Limited"},
	{"620 01", "SPACEFON"},
	{"620 02", "Ghana Telecom Mobile"},
	{"620 03", "MOBITEL"},
	{"621 20", "Econet Wireless Nigeria"},
	{"621 30", "MTN Nigeria"},
	{"621 40", "Mtel"},
	{"621 50", "Glo Mobile"},
	{"622 01", "CELTEL"},
	{"622 02", "Libertis"},
	{"624 01", "MTN Cameroon Ltd"},
	{"624 02", "Orange"},
	{"625 01", "CVMOVEL"},
	{"626 01", "CSTmovel"},
	{"627 01", "GETESA"},
	{"628 01", "LIBERTIS S.A."},
	{"628 02", "Telecel Gabon SA"},
	{"628 03", "Celtel Gabon SA"},
	{"629 01", "CelTel Congo SA"},
	{"629 10", "Libertis Telecom"},
	{"630 01", "VODACOM CONGO"},
	{"630 02", "CelTel Congo SA"},
	{"630 04", "CELLCO"},
	{"630 89", "OASIS"},
	{"631 02", "UNITEL"},
	{"633 01", "Cable & Wireless (Seychelles)"},
	{"633 10", "AIRTEL"},
	{"634 01", "MobiTel"},
	{"635 10", "MTN Rwandacell"},
	{"636 01", "ETMTN"},
	{"637 01", "TELESOM"},
	{"637 10", "Nationlink"},
	{"637 82", "Telsom Mobile"},
	{"638 01", "Evatis"},
	{"639 02", "Safaricom"},
	{"639 03", "KenCell"},
	{"640 02", "MOBITEL"},
	{"640 03", "ZANTEL"},
	{"640 04", "Vodacom Tanzania Ltd"},
	{"640 05", "Celtel Tanzania"},
	{"641 01", "CelTel Cellular"},
	{"641 10", "MTN-Uganda"},
	{"641 11", "mango"},
	{"642 01", "Spacetel - Burundi"},
	{"642 02", "SAFARIS"},
	{"643 01", "mCel"},
	{"643 04", "Vodacom Mozambique"},
	{"645 01", "CELTEL"},
	{"645 02", "Telecel Zambia Limited"},
	{"646 01", "Madacom"},
	{"646 02", "ANTARIS"},
	{"647 00", "Orange Reunion"},
	{"647 02", "Outremer Telecom"},
	{"647 10", "SFR REUNION"},
	{"648 01", "Net*One Cellular (Pvt) Ltd"},
	{"648 03", "TELECEL"},
	{"648 04", "Econet"},
	{"649 01", "MTC"},
	{"650 01", "Callpoint 900"},
	{"650 10", "CelTel Limited"},
	{"651 01", "Vodacom Lesotho (Pty) Ltd"},
	{"651 02", "Econet Ezi-Cel"},
	{"652 01", "MASCOM"},
	{"652 02", "ORANGE"},
	{"653 10", "Swazi MTN Limited"},
	{"654 00", "HURI"},
	{"655 01", "VodaCom"},
	{"655 07", "Cell C"},
	{"655 10", "MTN"},
	{"702 67", "Belize Telecommunications Ltd"},
	{"702 68", "International Telecommunication Limited (INTELCO)"},
	{"706 01", "CTE Telecom Personal SA de CV"},
	{"706 02", "Digicel"},
	{"710 21", "ENITEL"},
	{"710 73", "SERCOM S.A."},
	{"712 01", "I.C.E."},
	{"714 01", "Cable & Wireless Panama"},
	{"716 10", "TIM Peru"},
	{"722 07", "UNIFON"},
	{"722 31", "CTI Movil"},
	{"722 34", "Personal"},
	{"722 35", "PORT-HABLE"},
	{"724 03", "TIM BRASIL"},
	{"724 05", "Albra Telecomunicacoes"},
	{"724 05", "Alecan Telecomunicacoes"},
	{"724 05", "Americel"},
	{"724 05", "ATL"},
	{"724 05", "Stemar"},
	{"724 05", "Telet"},
	{"724 05", "Tess"},
	{"724 31", "TNL PCS S.A."},
	{"730 01", "Entel PCS"},
	{"730 02", "TELEFONICA"},
	{"730 10", "Entel PCS"},
	{"732 10", "Colombia Movil SA"},
	{"732 10", "Occidente y Caribe Celular SA Occel SA"},
	{"734 01", "Infonet"},
	{"734 02", "DIGITEL"},
	{"734 03", "DIGICEL C.A."},
	{"736 01", "Nuevatel PCS De Bolivia SA"},
	{"736 02", "Entel SA"},
	{"740 01", "Conecel S.A. (Consorcio Ecuatoriano de Telecomunicaciones S.A.)"},
	{"744 01", "VOX"},
	{"744 02", "Hutchison Telecommunications Paraguay S.A"},
	{"744 04", "Telecel Paraguay"},
	{"746 02", "TELESUR.GSM"},

	{ NULL, NULL }
};

GNOKII_API char *gn_network_name_get(char *network_code)
{
	int index = 0;

	while (networks[index].code &&
	       strncmp(networks[index].code, network_code, 6)) index++;

	/* for now be compacrapatible ;) */
	return networks[index].name ? networks[index].name : _("unknown");
}

GNOKII_API char *gn_network_code_get(char *network_name)
{
	int index = 0;

	while (networks[index].name &&
	       strcasecmp(networks[index].name, network_name)) index++;

	return networks[index].code ? networks[index].code : _("undefined");
}

GNOKII_API char *gn_network_code_find(char *network_name, char *country_name)
{
	int index = 0;
	char country_code[5];
	
	snprintf(country_code, 4, "%3s ", gn_country_code_get(country_name));
	country_code[4] = 0;
	while (networks[index].name &&
	       (!strstr(networks[index].code, country_code) ||
	        strcasecmp(networks[index].name, network_name))) index++;

	return networks[index].code ? networks[index].code : _("undefined");
}

GNOKII_API char *gn_country_name_get(char *country_code)
{
	int index = 0;

	while (countries[index].code &&
	       strncmp(countries[index].code, country_code, 3)) index++;

	return countries[index].name ? countries[index].name : _("unknown");
}

GNOKII_API char *gn_country_code_get(char *country_name)
{
	int index = 0;

	while (countries[index].name &&
	       strcasecmp(countries[index].name, country_name)) index++;

	return countries[index].code ? countries[index].code : _("undefined");
}

GNOKII_API bool gn_network_get(gn_network *network, int index)
{
	if (index < 0 || index >= ARRAY_LEN(networks) - 1)
		return false;
	*network = networks[index];
	return true;
}

GNOKII_API bool gn_country_get(gn_country *country, int index)
{
	if (index < 0 || index >= ARRAY_LEN(countries) - 1)
		return false;
	*country = countries[index];
	return true;
}

GNOKII_API char *gn_network2country(char *network_code)
{
	char ccode[4];
	
	snprintf(ccode, sizeof(ccode), "%s", network_code);

	return gn_country_name_get(ccode);
}
