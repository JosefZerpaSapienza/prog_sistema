/* PROGRAM 5-3 STARTS HERE AND ENDS WHERE NOTED BELOW. */

/* InitUnFp.c */

/* These functions maintain UNIX-style permissions
		in a Win32 (Windows NT only) SECURITY_ATTRIBUTES structure.
	There are three entries:
		InitializeUnixSA: Allocate & initialize structure.
		ChangeFilePermissions: Modify file's security.
		ReadFilePermissions: Interrogate a file's security.

	There are a number of assumptions and limitations:
		You can only modify and read a structure previously
			created by InitializeUnixSA.
		All permissions are the UNIX style:
			[R,W,X] - [User, Group, Other].
		The group is taken from the Group SID of the process.
			This may really represent several groups.
			You can, however, apply generic rights to ANY securable object.
		The change and read entries require the HANDLE of the
			object and obtain its SA. */

#include "EvryThng.h"

#define ACL_SIZE 1024
#define INIT_EXCEPTION 0x3
#define CHANGE_EXCEPTION 0x4
#define SID_SIZE LUSIZE  /* See support.h */
#define DOM_SIZE LUSIZE

static VOID FindGroup (DWORD, LPTSTR);

LPSECURITY_ATTRIBUTES InitializeUnixSA (DWORD UnixPerms,
		LPTSTR UsrNam, LPTSTR GrpNam, LPDWORD AceMasks, 
		LPHANDLE pHeap)

/* Allocate a structure and set the UNIX style permissions
	as specified in UnixPerms, which is 9-bits
	(low-order end) giving the required [R,W,X] settings
	for [User,Group,Other] in the familiar UNIX form. 
	Return a pointer to a security attributes structure and a pointer
	to a heap, which can be destroyed by the calling program to release
	all the data structres when it is finished with the security structure. */

{
	HANDLE SAHeap = HeapCreate (HEAP_GENERATE_EXCEPTIONS, 0, 0);
	/*  Several memory allocations are necessary to build the SA
		and they are all constructed in this heap. The structures
		are:
			1.	The SA itself
			2.	The SD
			3.	Three SIDs (for user, group, and everyone)
		This memory MUST be available to the calling program and must
		not be allocated on the stack of this function.	*/

	LPSECURITY_ATTRIBUTES pSA = NULL;
	PSECURITY_DESCRIPTOR pSD = NULL;
	PACL pAcl = NULL;
	BOOL Success, ok = TRUE;
	DWORD iBit, iSid, UsrCnt = ACCT_NAME_SIZE;

	/* Various tables of User, Group, and Everyone Names, SIDs,
		and so on for use first in LookupAccountName and SID creation. */

	LPTSTR GrpNms [3] = {EMPTY, EMPTY, _T ("Everyone")};
	PSID pSidTable [3] = {NULL, NULL, NULL};
	SID_NAME_USE sNamUse [] = 
		{SidTypeUser, SidTypeGroup, SidTypeWellKnownGroup};
	TCHAR RefDomain [3] [DOM_SIZE];
	DWORD RefDomCnt [3] = {DOM_SIZE, DOM_SIZE, DOM_SIZE};
	DWORD SidCnt [3] = {SID_SIZE, SID_SIZE, SID_SIZE};

__try {
	/* This is in a try-except block so as to
		free resources in case of any subsequent failure. */
	/*  The security structures must be allocated from the
		stack as it is used outside the function. */
	*pHeap = SAHeap;
	/*  The heap is enabled for HEAP_GENERATE_EXCEPTIONS, so the flag
		is not set in the individual HeapAlloc calls */
	pSA = HeapAlloc (SAHeap, 0, sizeof (SECURITY_ATTRIBUTES));
	pSA->nLength = sizeof (SECURITY_ATTRIBUTES);
	pSA->bInheritHandle = FALSE; /* Programmer can set this later. */
	
	pSD = HeapAlloc (SAHeap, 0, sizeof (SECURITY_DESCRIPTOR));
	pSA->lpSecurityDescriptor = pSD;
	if (!InitializeSecurityDescriptor (pSD, SECURITY_DESCRIPTOR_REVISION))
		ReportException (_T ("I.S.D. Error"), 21);

	/* Set up the table names for the user and group.
		Then get a SID for User, Group, and Everyone. */

	GrpNms[0] = UsrNam;
	if (GrpNam == NULL || _tcslen(GrpNam) == 0) { 
		/*  No group name specified. Get the user's primary group. */
		FindGroup (2, GrpNms [1]);
	} else GrpNms[1] = GrpNam;

	printf ("Group Name: %s\n", GrpNms [1]);


	/* Look up the three names, creating the SIDs. */

	for (iSid = 0; iSid < 3; iSid++) {
		pSidTable [iSid] = HeapAlloc (SAHeap, 0, SID_SIZE);
		ok = ok && LookupAccountName (NULL, GrpNms [iSid],
				pSidTable [iSid], &SidCnt [iSid],
				RefDomain [iSid], &RefDomCnt [iSid], &sNamUse [iSid]);
	}
	if (!ok)
		ReportException (_T("LookupAccntName Error"), 22);

	/* Set the security descriptor owner & group SIDs. */

	if (!SetSecurityDescriptorOwner (pSD, pSidTable [0], FALSE))
		ReportException (_T ("S.S.D.O. Error"), 23);
	if (!SetSecurityDescriptorGroup (pSD, pSidTable [1], FALSE))
		ReportException (_T ("S.S.D.G. Error"), 24);

	/* Allocate a structure for the ACL. */

	pAcl = HeapAlloc (SAHeap, 0, ACL_SIZE);

	/* Initialize an ACL. */

	if (!InitializeAcl (pAcl, ACL_SIZE, ACL_REVISION))
		ReportException (_T ("Initialize ACL Error"), 25);

	/* Add all the ACEs. Scan the permission bits, adding an allowed ACE when
		the bit is set and a denied ACE when the bit is reset. */

	Success = TRUE;
	for (iBit = 0; iBit < 9; iBit++) {
		if ((UnixPerms >> (8 - iBit) & 0x1) != 0 && AceMasks [iBit % 3] != 0)
			Success = Success && AddAccessAllowedAce (pAcl, ACL_REVISION,
					AceMasks [iBit % 3], pSidTable [iBit / 3]);
		else if (AceMasks [iBit % 3] != 0)
			Success = Success && AddAccessDeniedAce (pAcl, ACL_REVISION,
					AceMasks [iBit % 3], pSidTable [iBit / 3]);
	}
	/* Add a final deny all to everyone ACE */
	Success = Success && AddAccessDeniedAce (pAcl, ACL_REVISION, 
					STANDARD_RIGHTS_ALL | SPECIFIC_RIGHTS_ALL, pSidTable[2]);
	if (!Success)
		ReportException (_T ("AddAce error"), 26);
	if (!IsValidAcl (pAcl))
		ReportException (_T ("Created bad ACL"), 27);

	/* The ACL is now complete. Associate it with the security descriptor. */

	if (!SetSecurityDescriptorDacl (pSD, TRUE, pAcl, FALSE))
		ReportException (_T ("S.S.D.Dacl error"), 28);
	if (!IsValidSecurityDescriptor (pSD))
		ReportException (_T ("Created bad SD"), 29);

}	/* End of __try-except block. */

__except (EXCEPTION_EXECUTE_HANDLER)
{
	/*  An exception occured and was reported. All memory allocated to
		create the security descriptor and attributes is freed with a
		single HeapDestroy so that the inividual elements do not need
		to be deallocated. */
	if (SAHeap != NULL)
		HeapDestroy (SAHeap);
	pSA = NULL;
}
	return pSA;
}

/* PROGRAM 7-3 ENDS HERE. */


/* PROGRAM 7-4 STARTS HERE AND ENDS WHERE NOTED BELOW. */

DWORD ReadFilePermissions (LPTSTR lpFileName, LPTSTR UsrNm, LPTSTR GrpNm)


/* Return the UNIX style permissions for a file. */
{
	PSECURITY_DESCRIPTOR pSD = NULL;
	DWORD LenNeeded, PBits, iAce;
	BOOL DaclF, AclDefF, OwnerDefF, GroupDefF;
	BYTE DAcl [ACL_SIZE];
	PACL pAcl = (PACL) &DAcl;
	ACL_SIZE_INFORMATION ASizeInfo;
	PACCESS_ALLOWED_ACE pAce;
	BYTE AType;
	HANDLE ProcHeap = GetProcessHeap ();
	PSID pOwnerSid, pGroupSid;
	TCHAR RefDomain [2] [DOM_SIZE];
	DWORD RefDomCnt [2] = {DOM_SIZE, DOM_SIZE};
	DWORD AcctSize [2] = {ACCT_NAME_SIZE, ACCT_NAME_SIZE};
	SID_NAME_USE sNamUse [] = {SidTypeUser, SidTypeGroup};

__try {
	/* Get the required size for the security descriptor. */

	GetFileSecurity (lpFileName, OWNER_SECURITY_INFORMATION |
			GROUP_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION,
			pSD, 0, &LenNeeded);

	/* Create a security descriptor. */

	pSD = HeapAlloc (ProcHeap, HEAP_GENERATE_EXCEPTIONS, LenNeeded);
	if (!GetFileSecurity (lpFileName, OWNER_SECURITY_INFORMATION |
			GROUP_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION,
			pSD, LenNeeded, &LenNeeded))
		ReportError (_T ("GetFileSecurity error"), 30, TRUE);
	
	if (!GetSecurityDescriptorDacl (pSD, &DaclF, &pAcl, &AclDefF))
		ReportError (_T ("GetSecurityDescriptorDacl error"), 31, TRUE);

	/* Get the number of ACEs in the ACL. */

	if (!GetAclInformation (pAcl, &ASizeInfo,
			sizeof (ACL_SIZE_INFORMATION), AclSizeInformation))
		ReportError (_T ("GetAclInformation error"), 32, TRUE);


	/* Get Each Ace. We know that this ACL was created by
		our InitializeUnixSA function, so the ACES are in the
		same order as the UNIX permission bits. */

	PBits = 0;
	for (iAce = 0; iAce < ASizeInfo.AceCount; iAce++) {
		GetAce (pAcl, iAce, &pAce);
		AType = pAce->Header.AceType;
		if (AType == ACCESS_ALLOWED_ACE_TYPE)
			PBits |= (0x1 << (8-iAce));
	}

	/* Find the name of the owner and owning group. */
	/* Find the SIDs first. */

	if (!GetSecurityDescriptorOwner (pSD, &pOwnerSid, &OwnerDefF))
		ReportException (_T ("GetSecurityDescOwner error"), 33);
	if (!GetSecurityDescriptorGroup (pSD, &pGroupSid, &GroupDefF))
		ReportException (_T ("GetSecurityDescGrp error"), 34);
	if (!LookupAccountSid (NULL, pOwnerSid, UsrNm, &AcctSize [0], RefDomain [0],
			&RefDomCnt [0], &sNamUse [0]))
		ReportException (_T ("LookUpAccountSid error"), 35);
	if (!LookupAccountSid (NULL, pGroupSid, GrpNm, &AcctSize [1], RefDomain [1],
			&RefDomCnt [1], &sNamUse [1]))
		ReportException (_T ("LookUpAccountSid error"), 36);
	
	HeapFree (ProcHeap, 0, pSD);
	return PBits;
	}

	__except (EXCEPTION_EXECUTE_HANDLER) {
		if (pSD != NULL)
			HeapFree (ProcHeap, 0, pSD);
		return 0XFFFFFFFF;
	}
}

/* PROGRAM 7-4 ENDS HERE.  */


/* PROGRAM 7-5 STARTS HERE AND ENDS WHERE NOTED BELOW */

BOOL ChangeFilePermissions (DWORD fPerm, LPTSTR FileName, LPDWORD AceMasks)

/* Change permissions in an existing file. The group is left unchanged. */

/* Strategy:
	1.	Obtain the existing security descriptor using
		the internal function ReadFilePermissions.
	2.	Create a security attribute for the owner and permission bits.
	3.	Extract the security descriptor.
	4.	Set the file security with the new descriptor. */
{
	TCHAR UsrNm [ACCT_NAME_SIZE], GrpNm [ACCT_NAME_SIZE];
	DWORD OldfPerm;
	LPSECURITY_ATTRIBUTES pSA;
	PSECURITY_DESCRIPTOR pSD = NULL;
	HANDLE hSecHeap;

	/* Assure that the file exists.	*/

	if (_taccess (FileName, 0) != 0)	/* File does not exist. */
		return FALSE;
	OldfPerm = ReadFilePermissions (FileName, UsrNm, GrpNm);
	pSA = InitializeUnixSA (fPerm, UsrNm, GrpNm, AceMasks, &hSecHeap);
	pSD = pSA->lpSecurityDescriptor;

	if (!SetFileSecurity (FileName, DACL_SECURITY_INFORMATION, pSD))
		ReportError (_T ("SetFileSecurity error"), 37, TRUE);
	HeapDestroy (hSecHeap);
	return TRUE;
}

/* PROGRAM 7-5 ENDS HERE  */


/* This code is a hint as to how to solve Exercise 8-2. */

static VOID FindGroup (DWORD GroupNumber, LPTSTR GroupName)
/*	Find a group name associated with the owning user
	of the current process. */

{
	TCHAR RefDomain [DOM_SIZE];
	DWORD RefDomCnt = DOM_SIZE, AcctSize  = ACCT_NAME_SIZE;
	SID_NAME_USE GroupSidType  = SidTypeGroup;
	HANDLE tHandle;
	TOKEN_GROUPS TokenG[20]; /* You need some space for this. */
	DWORD TISize;

	if (!OpenProcessToken (GetCurrentProcess(), TOKEN_ALL_ACCESS, &tHandle))
		ReportError (_T ("OpenProcessToken error"), 0, TRUE);
	if (!GetTokenInformation (tHandle, TokenGroups,
			&TokenG, sizeof (TokenG), &TISize)) {
		ReportError (_T ("GetTokenInfo error"), 0, TRUE);
	}

	/* Experimentation shows that the groups entered are
		as follows:
		0	 -	None
		1	 -	Everyone
		2	 -	The first non-trivial group
	 	3,.. -	Keep looking up to the count, which is part
				of the structure - see the documentation! */

	if (!LookupAccountSid (NULL, TokenG[0].Groups[GroupNumber].Sid,
			GroupName, &AcctSize, RefDomain, &RefDomCnt, &GroupSidType))
			ReportError (_T("Error looking up Account Name"), 0, TRUE);
	return;
}
