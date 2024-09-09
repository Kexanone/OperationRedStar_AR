//------------------------------------------------------------------------------------------------
class ORS_MissionHeader : SCR_MissionHeader
{
	[Attribute(defvalue: "USSR", desc: "Faction key of the player faction")]
	FactionKey m_sPlayerFactionKey;

	[Attribute(defvalue: "US", desc: "Faction key of the enemy faction")]
	FactionKey m_sEnemyFactionKey;
	
	[Attribute(defvalue: "50", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(EAISkill), desc: "Enemy AI Skill Level")]
	EAISkill m_eEnemyAISkill;
	
	protected static ref ORS_MissionHeader s_pInstance;
	protected static ResourceName DEFAULT_MISSION_HEADER_CONFIG = "{8A819D80DA034565}Missions/ORS_Arland_Blufor.conf";
	
	//------------------------------------------------------------------------------------------------
	static ORS_MissionHeader GetInstance()
	{
		if (!s_pInstance)
		{
			s_pInstance = ORS_MissionHeader.Cast(GetGame().GetMissionHeader());
			
			if (!s_pInstance)
			{
				s_pInstance = SCR_ConfigHelperT<ORS_MissionHeader>.GetConfigObject(DEFAULT_MISSION_HEADER_CONFIG);
				s_pInstance.m_sSaveFileName = FilePath.StripPath(FilePath.StripExtension(GetGame().GetWorldFile()));
			}
		}
		
		return s_pInstance;
	}
}
