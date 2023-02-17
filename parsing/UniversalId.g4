grammar UniversalId;

// ANTLR 4 parser grammer
// _this is experimental translation based on ABNF notation from ISO 19848 annex C_

universalid: namingentity shipid localid;
shipid: SLASH IMO DIGIT+;
localid : namingrule localdataname;
namingentity : DATA PERIOD DNV PERIOD COM;

// Note: Subject to agreement and conformance with the "dnv-v2" naming rule,
// different URLs may be used as NamingEntity.

// <NamingRule> defines the rules for composing the <LocalDataName>.
namingrule : SLASH namingruleidentifier DASH namingruleversion; // here: /dnv-v2
namingruleidentifier : DNV;
namingruleversion : V2;

// Syntax specific to naming rule /dnv-v2
localdataname : viselement primaryitem ((SLASH SEC) secondaryitem)? (description)? (SLASH META) tagelement+;

// <VISElement> identifies the version of VIS used to compose <ItemPath> and <Description>.
viselement : (SLASH VIS DASH) visrelease;

// <PrimaryItem> describes an item (function or product) on board a ship that the
// data channel is assigned to or associated with. The <PrimaryItem> is considered the
// primary reference to Gmod, and is normally the item being monitored by the data channel.
primaryitem : itempath;

// <SecondaryItem> describes another item (function or product) on board a ship that is
// also related to the data channel. <SecondaryItem> provides additional description and
// can be used to disambiguate the <PrimaryItem> if there are multiple items of the same
// kind as <PrimaryItem>. Normally, <SecondaryItem> is an item being served by the
// <PrimaryItem>.
// <SecondaryItem> can not be an ancestor of the <PrimaryItem> in the Gmod hierarchy.
secondaryitem : itempath;

// <ItemPath> is a reference to an item (function or product) in the Gmod hierarchy.
itempath : functionelement+ (productelement)?;
functionelement : SLASH functioncode (DASH location)?;
productelement : SLASH productcode;

// <Description> provides a human readable description of the
// <PrimaryItem> and the <SecondaryItem>.
// NOTE: <Description> must be removed from a normalized channel ID.
description : ((SLASH TILDE) unreserved*)+;

// <TagElement>s provide additional information about the data channel.
// A tag may have a standardized value from a codebook or a custom value.
tagelement : SLASH tag ((DASH codebookvalue) | (TILDE customvalue));
tag : QTY | CNT | CALC | STATE | CMD | TYPE | POS | DETAIL;
codebookvalue : unreserved+;
customvalue : unreserved+;
// Valid values for the following elements are specified in VIS:
visrelease : unreserved+;
productcode : (VIS_ALPHA | DIGIT)+;
functioncode : ((productcode PERIOD) | DIGIT) (VIS_ALPHA | DIGIT | PERIOD)+;
location : unreserved+;
// Unreserved characters as per RFC 3986
unreserved : ALPHA | DIGIT | DASH | PERIOD | UNDERSCORE | TILDE;

////////////////////////////////////////////////////////////////////////////////////////////
// Lexer grammer
////////////////////////////////////////////////////////////////////////////////////////////

DATA: 'data';
DNV : 'dnv';
COM: 'com';
V2: 'v2';
VIS: 'vis';

IMO: 'IMO';

SEC: 'sec';
META: 'meta';

SLASH: '/';
DASH: '-';
PERIOD: '.';
TILDE: '~';
UNDERSCORE: '_';

QTY: 'qty';
CNT: 'cnt';
CALC: 'calc';
STATE: 'state';
CMD: 'cmd';
TYPE: 'type';
POS: 'pos';
DETAIL: 'detail';

VIS_ALPHA : [a-zA-Z];
ALPHA : [a-z];
DIGIT : [0-9];
