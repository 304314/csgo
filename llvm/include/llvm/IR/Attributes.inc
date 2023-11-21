------------- Classes -----------------
class Attr<string Attr:S = ?, list<AttrProperty> Attr:P = ?> {
  string AttrString = Attr:S;
  list<AttrProperty> Properties = Attr:P;
}
class AttrProperty {
}
class CompatRule<string CompatRule:F = ?> {
  string CompatFunc = CompatRule:F;
}
class EnumAttr<string EnumAttr:S = ?, list<AttrProperty> EnumAttr:P = ?> {	// Attr
  string AttrString = EnumAttr:S;
  list<AttrProperty> Properties = EnumAttr:P;
}
class IntAttr<string IntAttr:S = ?, list<AttrProperty> IntAttr:P = ?> {	// Attr
  string AttrString = IntAttr:S;
  list<AttrProperty> Properties = IntAttr:P;
}
class MergeRule<string MergeRule:F = ?> {
  string MergeFunc = MergeRule:F;
}
class StrBoolAttr<string StrBoolAttr:S = ?> {	// Attr
  string AttrString = StrBoolAttr:S;
  list<AttrProperty> Properties = [];
}
class TypeAttr<string TypeAttr:S = ?, list<AttrProperty> TypeAttr:P = ?> {	// Attr
  string AttrString = TypeAttr:S;
  list<AttrProperty> Properties = TypeAttr:P;
}
------------- Defs -----------------
def Alignment {	// Attr IntAttr
  string AttrString = "align";
  list<AttrProperty> Properties = [ParamAttr, RetAttr];
}
def AllocAlign {	// Attr EnumAttr
  string AttrString = "allocalign";
  list<AttrProperty> Properties = [ParamAttr];
}
def AllocKind {	// Attr IntAttr
  string AttrString = "allockind";
  list<AttrProperty> Properties = [FnAttr];
}
def AllocSize {	// Attr IntAttr
  string AttrString = "allocsize";
  list<AttrProperty> Properties = [FnAttr];
}
def AllocatedPointer {	// Attr EnumAttr
  string AttrString = "allocptr";
  list<AttrProperty> Properties = [ParamAttr];
}
def AlwaysInline {	// Attr EnumAttr
  string AttrString = "alwaysinline";
  list<AttrProperty> Properties = [FnAttr];
}
def ApproxFuncFPMath {	// Attr StrBoolAttr
  string AttrString = "approx-func-fp-math";
  list<AttrProperty> Properties = [];
}
def ArgMemOnly {	// Attr EnumAttr
  string AttrString = "argmemonly";
  list<AttrProperty> Properties = [FnAttr];
}
def Builtin {	// Attr EnumAttr
  string AttrString = "builtin";
  list<AttrProperty> Properties = [FnAttr];
}
def ByRef {	// Attr TypeAttr
  string AttrString = "byref";
  list<AttrProperty> Properties = [ParamAttr];
}
def ByVal {	// Attr TypeAttr
  string AttrString = "byval";
  list<AttrProperty> Properties = [ParamAttr];
}
def Cold {	// Attr EnumAttr
  string AttrString = "cold";
  list<AttrProperty> Properties = [FnAttr];
}
def Convergent {	// Attr EnumAttr
  string AttrString = "convergent";
  list<AttrProperty> Properties = [FnAttr];
}
def Dereferenceable {	// Attr IntAttr
  string AttrString = "dereferenceable";
  list<AttrProperty> Properties = [ParamAttr, RetAttr];
}
def DereferenceableOrNull {	// Attr IntAttr
  string AttrString = "dereferenceable_or_null";
  list<AttrProperty> Properties = [ParamAttr, RetAttr];
}
def DisableSanitizerInstrumentation {	// Attr EnumAttr
  string AttrString = "disable_sanitizer_instrumentation";
  list<AttrProperty> Properties = [FnAttr];
}
def ElementType {	// Attr TypeAttr
  string AttrString = "elementtype";
  list<AttrProperty> Properties = [ParamAttr];
}
def FnAttr {	// AttrProperty
}
def FnRetThunkExtern {	// Attr EnumAttr
  string AttrString = "fn_ret_thunk_extern";
  list<AttrProperty> Properties = [FnAttr];
}
def Hot {	// Attr EnumAttr
  string AttrString = "hot";
  list<AttrProperty> Properties = [FnAttr];
}
def ImmArg {	// Attr EnumAttr
  string AttrString = "immarg";
  list<AttrProperty> Properties = [ParamAttr];
}
def InAlloca {	// Attr TypeAttr
  string AttrString = "inalloca";
  list<AttrProperty> Properties = [ParamAttr];
}
def InReg {	// Attr EnumAttr
  string AttrString = "inreg";
  list<AttrProperty> Properties = [ParamAttr, RetAttr];
}
def InaccessibleMemOnly {	// Attr EnumAttr
  string AttrString = "inaccessiblememonly";
  list<AttrProperty> Properties = [FnAttr];
}
def InaccessibleMemOrArgMemOnly {	// Attr EnumAttr
  string AttrString = "inaccessiblemem_or_argmemonly";
  list<AttrProperty> Properties = [FnAttr];
}
def InlineHint {	// Attr EnumAttr
  string AttrString = "inlinehint";
  list<AttrProperty> Properties = [FnAttr];
}
def JumpTable {	// Attr EnumAttr
  string AttrString = "jumptable";
  list<AttrProperty> Properties = [FnAttr];
}
def LessPreciseFPMAD {	// Attr StrBoolAttr
  string AttrString = "less-precise-fpmad";
  list<AttrProperty> Properties = [];
}
def MinSize {	// Attr EnumAttr
  string AttrString = "minsize";
  list<AttrProperty> Properties = [FnAttr];
}
def MustProgress {	// Attr EnumAttr
  string AttrString = "mustprogress";
  list<AttrProperty> Properties = [FnAttr];
}
def Naked {	// Attr EnumAttr
  string AttrString = "naked";
  list<AttrProperty> Properties = [FnAttr];
}
def Nest {	// Attr EnumAttr
  string AttrString = "nest";
  list<AttrProperty> Properties = [ParamAttr];
}
def NoAlias {	// Attr EnumAttr
  string AttrString = "noalias";
  list<AttrProperty> Properties = [ParamAttr, RetAttr];
}
def NoBuiltin {	// Attr EnumAttr
  string AttrString = "nobuiltin";
  list<AttrProperty> Properties = [FnAttr];
}
def NoCallback {	// Attr EnumAttr
  string AttrString = "nocallback";
  list<AttrProperty> Properties = [FnAttr];
}
def NoCapture {	// Attr EnumAttr
  string AttrString = "nocapture";
  list<AttrProperty> Properties = [ParamAttr];
}
def NoCfCheck {	// Attr EnumAttr
  string AttrString = "nocf_check";
  list<AttrProperty> Properties = [FnAttr];
}
def NoDuplicate {	// Attr EnumAttr
  string AttrString = "noduplicate";
  list<AttrProperty> Properties = [FnAttr];
}
def NoFree {	// Attr EnumAttr
  string AttrString = "nofree";
  list<AttrProperty> Properties = [FnAttr, ParamAttr];
}
def NoImplicitFloat {	// Attr EnumAttr
  string AttrString = "noimplicitfloat";
  list<AttrProperty> Properties = [FnAttr];
}
def NoInfsFPMath {	// Attr StrBoolAttr
  string AttrString = "no-infs-fp-math";
  list<AttrProperty> Properties = [];
}
def NoInline {	// Attr EnumAttr
  string AttrString = "noinline";
  list<AttrProperty> Properties = [FnAttr];
}
def NoInlineLineTables {	// Attr StrBoolAttr
  string AttrString = "no-inline-line-tables";
  list<AttrProperty> Properties = [];
}
def NoJumpTables {	// Attr StrBoolAttr
  string AttrString = "no-jump-tables";
  list<AttrProperty> Properties = [];
}
def NoMerge {	// Attr EnumAttr
  string AttrString = "nomerge";
  list<AttrProperty> Properties = [FnAttr];
}
def NoNansFPMath {	// Attr StrBoolAttr
  string AttrString = "no-nans-fp-math";
  list<AttrProperty> Properties = [];
}
def NoProfile {	// Attr EnumAttr
  string AttrString = "noprofile";
  list<AttrProperty> Properties = [FnAttr];
}
def NoRecurse {	// Attr EnumAttr
  string AttrString = "norecurse";
  list<AttrProperty> Properties = [FnAttr];
}
def NoRedZone {	// Attr EnumAttr
  string AttrString = "noredzone";
  list<AttrProperty> Properties = [FnAttr];
}
def NoReturn {	// Attr EnumAttr
  string AttrString = "noreturn";
  list<AttrProperty> Properties = [FnAttr];
}
def NoSanitizeBounds {	// Attr EnumAttr
  string AttrString = "nosanitize_bounds";
  list<AttrProperty> Properties = [FnAttr];
}
def NoSanitizeCoverage {	// Attr EnumAttr
  string AttrString = "nosanitize_coverage";
  list<AttrProperty> Properties = [FnAttr];
}
def NoSignedZerosFPMath {	// Attr StrBoolAttr
  string AttrString = "no-signed-zeros-fp-math";
  list<AttrProperty> Properties = [];
}
def NoSync {	// Attr EnumAttr
  string AttrString = "nosync";
  list<AttrProperty> Properties = [FnAttr];
}
def NoUndef {	// Attr EnumAttr
  string AttrString = "noundef";
  list<AttrProperty> Properties = [ParamAttr, RetAttr];
}
def NoUnwind {	// Attr EnumAttr
  string AttrString = "nounwind";
  list<AttrProperty> Properties = [FnAttr];
}
def NonLazyBind {	// Attr EnumAttr
  string AttrString = "nonlazybind";
  list<AttrProperty> Properties = [FnAttr];
}
def NonNull {	// Attr EnumAttr
  string AttrString = "nonnull";
  list<AttrProperty> Properties = [ParamAttr, RetAttr];
}
def NullPointerIsValid {	// Attr EnumAttr
  string AttrString = "null_pointer_is_valid";
  list<AttrProperty> Properties = [FnAttr];
}
def OptForFuzzing {	// Attr EnumAttr
  string AttrString = "optforfuzzing";
  list<AttrProperty> Properties = [FnAttr];
}
def OptimizeForSize {	// Attr EnumAttr
  string AttrString = "optsize";
  list<AttrProperty> Properties = [FnAttr];
}
def OptimizeNone {	// Attr EnumAttr
  string AttrString = "optnone";
  list<AttrProperty> Properties = [FnAttr];
}
def ParamAttr {	// AttrProperty
}
def Preallocated {	// Attr TypeAttr
  string AttrString = "preallocated";
  list<AttrProperty> Properties = [FnAttr, ParamAttr];
}
def PresplitCoroutine {	// Attr EnumAttr
  string AttrString = "presplitcoroutine";
  list<AttrProperty> Properties = [FnAttr];
}
def ProfileSampleAccurate {	// Attr StrBoolAttr
  string AttrString = "profile-sample-accurate";
  list<AttrProperty> Properties = [];
}
def ReadNone {	// Attr EnumAttr
  string AttrString = "readnone";
  list<AttrProperty> Properties = [FnAttr, ParamAttr];
}
def ReadOnly {	// Attr EnumAttr
  string AttrString = "readonly";
  list<AttrProperty> Properties = [FnAttr, ParamAttr];
}
def RetAttr {	// AttrProperty
}
def Returned {	// Attr EnumAttr
  string AttrString = "returned";
  list<AttrProperty> Properties = [ParamAttr];
}
def ReturnsTwice {	// Attr EnumAttr
  string AttrString = "returns_twice";
  list<AttrProperty> Properties = [FnAttr];
}
def SExt {	// Attr EnumAttr
  string AttrString = "signext";
  list<AttrProperty> Properties = [ParamAttr, RetAttr];
}
def SafeStack {	// Attr EnumAttr
  string AttrString = "safestack";
  list<AttrProperty> Properties = [FnAttr];
}
def SanitizeAddress {	// Attr EnumAttr
  string AttrString = "sanitize_address";
  list<AttrProperty> Properties = [FnAttr];
}
def SanitizeHWAddress {	// Attr EnumAttr
  string AttrString = "sanitize_hwaddress";
  list<AttrProperty> Properties = [FnAttr];
}
def SanitizeMemTag {	// Attr EnumAttr
  string AttrString = "sanitize_memtag";
  list<AttrProperty> Properties = [FnAttr];
}
def SanitizeMemory {	// Attr EnumAttr
  string AttrString = "sanitize_memory";
  list<AttrProperty> Properties = [FnAttr];
}
def SanitizeThread {	// Attr EnumAttr
  string AttrString = "sanitize_thread";
  list<AttrProperty> Properties = [FnAttr];
}
def ShadowCallStack {	// Attr EnumAttr
  string AttrString = "shadowcallstack";
  list<AttrProperty> Properties = [FnAttr];
}
def Speculatable {	// Attr EnumAttr
  string AttrString = "speculatable";
  list<AttrProperty> Properties = [FnAttr];
}
def SpeculativeLoadHardening {	// Attr EnumAttr
  string AttrString = "speculative_load_hardening";
  list<AttrProperty> Properties = [FnAttr];
}
def StackAlignment {	// Attr IntAttr
  string AttrString = "alignstack";
  list<AttrProperty> Properties = [FnAttr, ParamAttr];
}
def StackProtect {	// Attr EnumAttr
  string AttrString = "ssp";
  list<AttrProperty> Properties = [FnAttr];
}
def StackProtectReq {	// Attr EnumAttr
  string AttrString = "sspreq";
  list<AttrProperty> Properties = [FnAttr];
}
def StackProtectStrong {	// Attr EnumAttr
  string AttrString = "sspstrong";
  list<AttrProperty> Properties = [FnAttr];
}
def StrictFP {	// Attr EnumAttr
  string AttrString = "strictfp";
  list<AttrProperty> Properties = [FnAttr];
}
def StructRet {	// Attr TypeAttr
  string AttrString = "sret";
  list<AttrProperty> Properties = [ParamAttr];
}
def SwiftAsync {	// Attr EnumAttr
  string AttrString = "swiftasync";
  list<AttrProperty> Properties = [ParamAttr];
}
def SwiftError {	// Attr EnumAttr
  string AttrString = "swifterror";
  list<AttrProperty> Properties = [ParamAttr];
}
def SwiftSelf {	// Attr EnumAttr
  string AttrString = "swiftself";
  list<AttrProperty> Properties = [ParamAttr];
}
def UWTable {	// Attr IntAttr
  string AttrString = "uwtable";
  list<AttrProperty> Properties = [FnAttr];
}
def UnsafeFPMath {	// Attr StrBoolAttr
  string AttrString = "unsafe-fp-math";
  list<AttrProperty> Properties = [];
}
def UseSampleProfile {	// Attr StrBoolAttr
  string AttrString = "use-sample-profile";
  list<AttrProperty> Properties = [];
}
def VScaleRange {	// Attr IntAttr
  string AttrString = "vscale_range";
  list<AttrProperty> Properties = [FnAttr];
}
def WillReturn {	// Attr EnumAttr
  string AttrString = "willreturn";
  list<AttrProperty> Properties = [FnAttr];
}
def WriteOnly {	// Attr EnumAttr
  string AttrString = "writeonly";
  list<AttrProperty> Properties = [FnAttr, ParamAttr];
}
def ZExt {	// Attr EnumAttr
  string AttrString = "zeroext";
  list<AttrProperty> Properties = [ParamAttr, RetAttr];
}
def anonymous_0 {	// CompatRule
  string CompatFunc = "isEqual<SanitizeAddressAttr>";
}
def anonymous_1 {	// CompatRule
  string CompatFunc = "isEqual<SanitizeThreadAttr>";
}
def anonymous_10 {	// MergeRule
  string MergeFunc = "setAND<NoInfsFPMathAttr>";
}
def anonymous_11 {	// MergeRule
  string MergeFunc = "setAND<NoNansFPMathAttr>";
}
def anonymous_12 {	// MergeRule
  string MergeFunc = "setAND<ApproxFuncFPMathAttr>";
}
def anonymous_13 {	// MergeRule
  string MergeFunc = "setAND<NoSignedZerosFPMathAttr>";
}
def anonymous_14 {	// MergeRule
  string MergeFunc = "setAND<UnsafeFPMathAttr>";
}
def anonymous_15 {	// MergeRule
  string MergeFunc = "setOR<NoImplicitFloatAttr>";
}
def anonymous_16 {	// MergeRule
  string MergeFunc = "setOR<NoJumpTablesAttr>";
}
def anonymous_17 {	// MergeRule
  string MergeFunc = "setOR<ProfileSampleAccurateAttr>";
}
def anonymous_18 {	// MergeRule
  string MergeFunc = "setOR<SpeculativeLoadHardeningAttr>";
}
def anonymous_19 {	// MergeRule
  string MergeFunc = "adjustCallerSSPLevel";
}
def anonymous_2 {	// CompatRule
  string CompatFunc = "isEqual<SanitizeMemoryAttr>";
}
def anonymous_20 {	// MergeRule
  string MergeFunc = "adjustCallerStackProbes";
}
def anonymous_21 {	// MergeRule
  string MergeFunc = "adjustCallerStackProbeSize";
}
def anonymous_22 {	// MergeRule
  string MergeFunc = "adjustMinLegalVectorWidth";
}
def anonymous_23 {	// MergeRule
  string MergeFunc = "adjustNullPointerValidAttr";
}
def anonymous_24 {	// MergeRule
  string MergeFunc = "setAND<MustProgressAttr>";
}
def anonymous_3 {	// CompatRule
  string CompatFunc = "isEqual<SanitizeHWAddressAttr>";
}
def anonymous_4 {	// CompatRule
  string CompatFunc = "isEqual<SanitizeMemTagAttr>";
}
def anonymous_5 {	// CompatRule
  string CompatFunc = "isEqual<SafeStackAttr>";
}
def anonymous_6 {	// CompatRule
  string CompatFunc = "isEqual<ShadowCallStackAttr>";
}
def anonymous_7 {	// CompatRule
  string CompatFunc = "isEqual<UseSampleProfileAttr>";
}
def anonymous_8 {	// CompatRule
  string CompatFunc = "isEqual<NoProfileAttr>";
}
def anonymous_9 {	// MergeRule
  string MergeFunc = "setAND<LessPreciseFPMADAttr>";
}
