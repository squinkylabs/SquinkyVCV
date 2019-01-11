/**
 * This is the minblep.cpp from VCVRack.
 * Moved here to make it easier to build.
 * TODO: don't do this - build from normal folder.
 */

// Need to make this compile in MS tools for unit tests
#if defined(_MSC_VER)
#define __attribute__(x)

#pragma warning(disable: 4244 4838 4305)
#endif

#include "SQMath.h"
//#include "dsp/minblep.hpp"


namespace rack {

#ifdef __V1
namespace dsp {
#endif


// TODO I should probably compute this on launch
const float minblep_16_32[] = {
	0.00011023, 0.00022416, 0.00034297, 0.00046661, 0.00059353, 0.00072403, 0.00085807, 0.00099544, 0.00113669, 0.00128244, 0.00143357, 0.00159180, 0.00175889, 0.00193681, 0.00212798, 0.00233618, 0.00256460, 0.00281804, 0.00310111, 0.00341981, 0.00378001, 0.00418855, 0.00465331, 0.00518249, 0.00578578, 0.00647247, 0.00725390, 0.00814134, 0.00914690, 0.01028436, 0.01156735, 0.01301055, 0.01462957, 0.01644073, 0.01846137, 0.02070885, 0.02320160, 0.02595890, 0.02900020, 0.03234551, 0.03601537, 0.04003051, 0.04441243, 0.04918219, 0.05436156, 0.05997159, 0.06603400, 0.07256979, 0.07960004, 0.08714473, 0.09522393, 0.10385663, 0.11306075, 0.12285375, 0.13325208, 0.14427035, 0.15592176, 0.16821821, 0.18117011, 0.19478568, 0.20907153, 0.22403158, 0.23966792, 0.25598019, 0.27296567, 0.29061884, 0.30893153, 0.32789305, 0.34748974, 0.36770493, 0.38851914, 0.40990984, 0.43185139, 0.45431516, 0.47726974, 0.50068015, 0.52450907, 0.54871643, 0.57325858, 0.59809023, 0.62316221, 0.64842403, 0.67382252, 0.69930243, 0.72480643, 0.75027585, 0.77565008, 0.80086774, 0.82586610, 0.85058230, 0.87495226, 0.89891225, 0.92239881, 0.94534874, 0.96769959, 0.98939002, 1.01036096, 1.03055346, 1.04991257, 1.06838441, 1.08591819, 1.10246634, 1.11798477, 1.13243258, 1.14577281, 1.15797329, 1.16900539, 1.17884552, 1.18747461, 1.19487906, 1.20105016, 1.20598388, 1.20968246, 1.21215260, 1.21340692, 1.21346366, 1.21234596, 1.21008229, 1.20670640, 1.20225751, 1.19677913, 1.19032025, 1.18293369, 1.17467666, 1.16561043, 1.15579987, 1.14531350, 1.13422263, 1.12260056, 1.11052382, 1.09806967, 1.08531761, 1.07234740, 1.05923963, 1.04607463, 1.03293216, 1.01989150, 1.00703001, 0.99442369, 0.98214602, 0.97026730, 0.95885533, 0.94797397, 0.93768352, 0.92803955, 0.91909295, 0.91089052, 0.90347338, 0.89687651, 0.89113057, 0.88625956, 0.88228178, 0.87920940, 0.87704903, 0.87580091, 0.87545884, 0.87601125, 0.87744081, 0.87972385, 0.88283205, 0.88673097, 0.89138156, 0.89674008, 0.90275854, 0.90938461, 0.91656262, 0.92423326, 0.93233514, 0.94080383, 0.94957352, 0.95857650, 0.96774524, 0.97700989, 0.98630202, 0.99555433, 1.00469887, 1.01367104, 1.02240634, 1.03084445, 1.03892660, 1.04659796, 1.05380774, 1.06050789, 1.06665611, 1.07221341, 1.07714641, 1.08142638, 1.08503044, 1.08793986, 1.09014225, 1.09163105, 1.09240448, 1.09246624, 1.09182608, 1.09049749, 1.08850145, 1.08586180, 1.08260751, 1.07877254, 1.07439494, 1.06951582, 1.06418049, 1.05843639, 1.05233502, 1.04592884, 1.03927267, 1.03242254, 1.02543569, 1.01836896, 1.01127994, 1.00422585, 0.99726236, 0.99044448, 0.98382431, 0.97745323, 0.97137886, 0.96564668, 0.96029860, 0.95537275, 0.95090336, 0.94692129, 0.94345307, 0.94051903, 0.93813735, 0.93632013, 0.93507481, 0.93440408, 0.93430620, 0.93477470, 0.93579876, 0.93736202, 0.93944514, 0.94202399, 0.94507092, 0.94855475, 0.95244062, 0.95669132, 0.96126628, 0.96612370, 0.97121888, 0.97650659, 0.98193920, 0.98746979, 0.99305087, 0.99863440, 1.00417352, 1.00962198, 1.01493454, 1.02006841, 1.02498233, 1.02963722, 1.03399694, 1.03802824, 1.04170084, 1.04498816, 1.04786694, 1.05031765, 1.05232465, 1.05387664, 1.05496621, 1.05558980, 1.05574775, 1.05544484, 1.05468917, 1.05349278, 1.05187225, 1.04984665, 1.04743862, 1.04467380, 1.04158032, 1.03818953, 1.03453457, 1.03065002, 1.02657270, 1.02234089, 1.01799285, 1.01356828, 1.00910676, 1.00464690, 1.00022852, 0.99588913, 0.99166596, 0.98759449, 0.98370826, 0.98003900, 0.97661632, 0.97346693, 0.97061497, 0.96808159, 0.96588528, 0.96404094, 0.96256018, 0.96145141, 0.96072024, 0.96036756, 0.96039182, 0.96078801, 0.96154761, 0.96265966, 0.96410930, 0.96587980, 0.96795046, 0.97029966, 0.97290313, 0.97573400, 0.97876412, 0.98196387, 0.98530257, 0.98874873, 0.99227041, 0.99583501, 0.99940956, 1.00296247, 1.00646222, 1.00987804, 1.01318014, 1.01634085, 1.01933396, 1.02213407, 1.02471924, 1.02706873, 1.02916455, 1.03099120, 1.03253567, 1.03378832, 1.03474081, 1.03538895, 1.03573155, 1.03576887, 1.03550470, 1.03494596, 1.03410137, 1.03298199, 1.03160226, 1.02997768, 1.02812684, 1.02606928, 1.02382588, 1.02142048, 1.01887643, 1.01621914, 1.01347411, 1.01066744, 1.00782573, 1.00497484, 1.00214148, 0.99935061, 0.99662775, 0.99399650, 0.99147928, 0.98909825, 0.98687279, 0.98482132, 0.98295969, 0.98130250, 0.97986293, 0.97865003, 0.97767186, 0.97693449, 0.97644144, 0.97619355, 0.97618985, 0.97642678, 0.97689945, 0.97759926, 0.97851688, 0.97964054, 0.98095751, 0.98245293, 0.98410982, 0.98591143, 0.98783875, 0.98987180, 0.99199086, 0.99417531, 0.99640393, 0.99865520, 1.00090814, 1.00314200, 1.00533652, 1.00747168, 1.00952899, 1.01149011, 1.01333869, 1.01505876, 1.01663721, 1.01806092, 1.01931858, 1.02040136, 1.02130210, 1.02201474, 1.02253604, 1.02286315, 1.02299595, 1.02293634, 1.02268875, 1.02225709, 1.02164853, 1.02087140, 1.01993537, 1.01885104, 1.01763213, 1.01629102, 1.01484251, 1.01330113, 1.01168346, 1.01000535, 1.00828362, 1.00653470, 1.00477552, 1.00302279, 1.00129271, 0.99960083, 0.99796313, 0.99639338, 0.99490535, 0.99351192, 0.99222440, 0.99105316, 0.99000835, 0.98909634, 0.98832458, 0.98769748, 0.98721880, 0.98689038, 0.98671335, 0.98668671, 0.98680866, 0.98707467, 0.98748106, 0.98802143, 0.98868805, 0.98947275, 0.99036646, 0.99135911, 0.99243897, 0.99359548, 0.99481559, 0.99608690, 0.99739677, 0.99873203, 1.00007975, 1.00142658, 1.00276041, 1.00406802, 1.00533855, 1.00655985, 1.00772119, 1.00881267, 1.00982487, 1.01075006, 1.01158035, 1.01230979, 1.01293266, 1.01344562, 1.01384568, 1.01412964, 1.01429844, 1.01435149, 1.01429081, 1.01411796, 1.01383793, 1.01345396, 1.01297212, 1.01239896, 1.01174057, 1.01100540, 1.01020169, 1.00933838, 1.00842452, 1.00746989, 1.00648463, 1.00547814, 1.00446069, 1.00344217, 1.00243223, 1.00143993, 1.00047517, 0.99954665, 0.99866235, 0.99782991, 0.99705631, 0.99634796, 0.99571103, 0.99514896, 0.99466711, 0.99426842, 0.99395454, 0.99372780, 0.99358785, 0.99353504, 0.99356812, 0.99368507, 0.99388355, 0.99415958, 0.99450946, 0.99492884, 0.99541146, 0.99595195, 0.99654400, 0.99718058, 0.99785507, 0.99856061, 0.99928933, 1.00003409, 1.00078654, 1.00154030, 1.00228751, 1.00302088, 1.00373411, 1.00442064, 1.00507402, 1.00568807, 1.00625861, 1.00678062, 1.00724947, 1.00766242, 1.00801611, 1.00830781, 1.00853622, 1.00870037, 1.00880003, 1.00883472, 1.00880599, 1.00871444, 1.00856256, 1.00835276, 1.00808787, 1.00777161, 1.00740683, 1.00699902, 1.00655174, 1.00606918, 1.00555754, 1.00502074, 1.00446463, 1.00389385, 1.00331378, 1.00272894, 1.00214446, 1.00156450, 1.00099480, 1.00043809, 0.99989861, 0.99938035, 0.99888563, 0.99841738, 0.99797773, 0.99756885, 0.99719167, 0.99684638, 0.99653435, 0.99625492, 0.99600822, 0.99579257, 0.99560666, 0.99544859, 0.99531645, 0.99520802, 0.99512035, 0.99504977, 0.99499327, 0.99494720, 0.99490863, 0.99487358, 0.99483842, 0.99479860, 0.99475139, 0.99469274, 0.99461985, 0.99452943, 0.99441826, 0.99428391, 0.99412394, 0.99393618, 0.99371916, 0.99347186, 0.99319351, 0.99288350, 0.99254173, 0.99216926, 0.99176681, 0.99133593, 0.99087846, 0.99039710, 0.98989451, 0.98937410, 0.98883879, 0.98829341, 0.98774183, 0.98718870, 0.98663867, 0.98609704, 0.98556888, 0.98505920, 0.98457366, 0.98411739, 0.98369551, 0.98331392, 0.98297703, 0.98268992, 0.98245704, 0.98228276, 0.98217118, 0.98212564, 0.98214996, 0.98224646, 0.98241693, 0.98266387, 0.98298812, 0.98339027, 0.98387039, 0.98442757, 0.98506147, 0.98577005, 0.98655069, 0.98740059, 0.98831660, 0.98929417, 0.99032903, 0.99141610, 0.99254966, 0.99372399, 0.99493235, 0.99616790, 0.99742448, 0.99869454, 0.99997079, 1.00124514, 1.00251019, 1.00375855, 1.00498211, 1.00617325, 1.00732577, 1.00843060, 1.00948179, 1.01047242, 1.01139629, 1.01224756, 1.01302052, 1.01371098, 1.01431394, 1.01482534, 1.01524329, 1.01556420, 1.01578641, 1.01590872, 1.01593041, 1.01585221, 1.01567483, 1.01539934, 1.01502872, 1.01456475, 1.01401234, 1.01337564, 1.01265824, 1.01186681, 1.01100624, 1.01008308, 1.00910449, 1.00807726, 1.00700867, 1.00590730, 1.00478077, 1.00363719, 1.00248516, 1.00133204, 1.00018704, 0.99905777, 0.99795246, 0.99687880, 0.99584419, 0.99485654, 0.99392235, 0.99304855, 0.99224019, 0.99150354, 0.99084306, 0.99026269, 0.98976678, 0.98935860, 0.98903960, 0.98881179, 0.98867643, 0.98863405, 0.98868364, 0.98882431, 0.98905468, 0.98937207, 0.98977262, 0.99025381, 0.99081081, 0.99143785, 0.99213088, 0.99288279, 0.99368858, 0.99454105, 0.99543440, 0.99635935, 0.99730903, 0.99827605, 0.99925363, 1.00023246, 1.00120509, 1.00216508, 1.00310433, 1.00401509, 1.00489199, 1.00572717, 1.00651467, 1.00724912, 1.00792384, 1.00853491, 1.00907779, 1.00954843, 1.00994480, 1.01026309, 1.01050198, 1.01066041, 1.01073647, 1.01073205, 1.01064730, 1.01048374, 1.01024330, 1.00992775, 1.00954151, 1.00908792, 1.00857103, 1.00799608, 1.00736785, 1.00669181, 1.00597465, 1.00522304, 1.00444114, 1.00363779, 1.00281966, 1.00199306, 1.00116563, 1.00034273, 0.99953192, 0.99873996, 0.99797446, 0.99724007, 0.99654233, 0.99588805, 0.99528110, 0.99472731, 0.99422991, 0.99379259, 0.99341941, 0.99311274, 0.99287349, 0.99270529, 0.99260652, 0.99257857, 0.99261945, 0.99273038, 0.99290955, 0.99315345, 0.99346066, 0.99382687, 0.99424839, 0.99472272, 0.99524361, 0.99580789, 0.99640882, 0.99704123, 0.99769914, 0.99837738, 0.99906975, 0.99976873, 1.00046957, 1.00116682, 1.00185359, 1.00252378, 1.00317132, 1.00379229, 1.00438106, 1.00493193, 1.00543988, 1.00590360, 1.00631797, 1.00667834, 1.00698411, 1.00723267, 1.00742185, 1.00755143, 1.00761986, 1.00762773, 1.00757504, 1.00746322, 1.00729394, 1.00706911, 1.00679159, 1.00646341, 1.00608873, 1.00567114, 1.00521481, 1.00472414, 1.00420344, 1.00365949, 1.00309455, 1.00251555, 1.00192726, 1.00133574, 1.00074565, 1.00016236, 0.99959171, 0.99903804, 0.99850535, 0.99799985, 0.99752414, 0.99708354, 0.99668151, 0.99632025, 0.99600381, 0.99573332, 0.99551266, 0.99534053, 0.99522054, 0.99515241, 0.99513555, 0.99517030, 0.99525541, 0.99538952, 0.99557215, 0.99579948, 0.99607027, 0.99638057, 0.99672699, 0.99710703, 0.99751633, 0.99795067, 0.99840581, 0.99887735, 0.99936110, 0.99985218, 1.00034654, 1.00083947, 1.00132573, 1.00180173, 1.00226319, 1.00270593, 1.00312614, 1.00351942, 1.00388277, 1.00421333, 1.00450921, 1.00476730, 1.00498581, 1.00516307, 1.00529838, 1.00539017, 1.00543821, 1.00544345, 1.00540483, 1.00532460, 1.00520289, 1.00504267, 1.00484514, 1.00461233, 1.00434721, 1.00405192, 1.00373054, 1.00338578, 1.00302088, 1.00264025, 1.00224662, 1.00184488, 1.00143826, 1.00103080, 1.00062573, 1.00022781, 0.99984139, 0.99946749, 0.99911129, 0.99877495, 0.99846184, 0.99817514, 0.99791592, 0.99768758, 0.99749118, 0.99732894, 0.99720013, 0.99710739, 0.99705017, 0.99702936, 0.99704379, 0.99709320, 0.99717599, 0.99729306, 0.99744141, 0.99761915, 0.99782479, 0.99805552, 0.99830848, 0.99858129, 0.99887139, 0.99917626, 0.99949169, 0.99981594, 1.00014460, 1.00047433, 1.00080299, 1.00112653, 1.00144279, 1.00174880, 1.00204086, 1.00231719, 1.00257576, 1.00281310, 1.00302708, 1.00321829, 1.00338340, 1.00352204, 1.00363243, 1.00371420, 1.00376749, 1.00379157, 1.00378597, 1.00375187, 1.00369036, 1.00360274, 1.00348926, 1.00335169, 1.00319159, 1.00301039, 1.00281143, 1.00259674, 1.00236738, 1.00212586, 1.00187516, 1.00161850, 1.00135744, 1.00109494, 1.00083435, 1.00057673, 1.00032485, 1.00008142, 0.99984884, 0.99962848, 0.99942255, 0.99923331, 0.99906200, 0.99890971, 0.99877805, 0.99866712, 0.99857920, 0.99851406, 0.99847245, 0.99845350, 0.99845672, 0.99848276, 0.99853104, 0.99859989, 0.99868929, 0.99879676, 0.99892145, 0.99906349, 0.99921906, 0.99938750, 0.99956673, 0.99975437, 0.99995005, 1.00015068, 1.00035346, 1.00055718, 1.00075948, 1.00095868, 1.00115299, 1.00134051, 1.00151980, 1.00168908, 1.00184536, 1.00198960, 1.00211966, 1.00223446, 1.00233328, 1.00241554, 1.00247967, 1.00252616, 1.00255466, 1.00256550, 1.00255883, 1.00253487, 1.00249410, 1.00243723, 1.00236428, 1.00227737, 1.00217772, 1.00206661, 1.00194490, 1.00181246, 1.00167227, 1.00152564, 1.00137448, 1.00121903, 1.00106311, 1.00090694, 1.00075161, 1.00059938, 1.00045121, 1.00030851, 1.00017321, 1.00004661, 0.99992871, 0.99982160, 0.99972528, 0.99964148, 0.99956852, 0.99950951, 0.99946433, 0.99943203, 0.99941361, 0.99940944, 0.99941790, 0.99943894, 0.99947232, 0.99951869, 0.99957561, 0.99964315, 0.99972034, 0.99980599, 0.99989998, 1.00000000
};

#ifdef __V1
}			// namespace dsp
#endif


} // namespace rack
