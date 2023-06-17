modded class SCR_BaseGameMode : BaseGameMode
{
	protected ref array<vector> m_ORS_DeletedEntityPositions = {};
	protected IEntity m_eORS_NearestEntity;
	protected vector m_ORS_SearchPos;
	protected float m_fORS_NearestDistanceSq;
	protected const float ORS_SEARCH_DISTANCE = 0.01;
	
	override void OnPlayerConnected(int playerId)
	{
		super.OnPlayerConnected(playerId);
		SCR_PlayerController deleteCtrl = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId));
		deleteCtrl.Rpc(deleteCtrl.ORS_DeleteInitialEntityPositions, m_ORS_DeletedEntityPositions);
	};
		
	void ORS_DeleteEntityPositionsGlobal(array<vector> entityPositions)
	{
		m_ORS_DeletedEntityPositions.InsertAll(entityPositions);
		ORS_DeleteEntityPositionsLocal(entityPositions);
		Rpc(ORS_DeleteEntityPositionsLocal, entityPositions);
	};
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void ORS_DeleteEntityPositionsLocal(array<vector> entityPositions)
	{
		foreach (vector pos : entityPositions)
		{
			IEntity entity = ORS_GetNearestEntity(pos);
			if (entity)
				SCR_EntityHelper.DeleteEntityAndChildren(entity);
		};
	};
	
	array<vector> ORS_GetDeletedEntityPositions()
	{
		return m_ORS_DeletedEntityPositions;
	};
	
	protected IEntity ORS_GetNearestEntity(vector pos)
	{
		m_ORS_SearchPos = pos;
		m_fORS_NearestDistanceSq = Math.Pow(ORS_SEARCH_DISTANCE, 2);
		m_eORS_NearestEntity = null;
		GetGame().GetWorld().QueryEntitiesBySphere(m_ORS_SearchPos, ORS_SEARCH_DISTANCE, ORS_EntitySearchCallback);
		return m_eORS_NearestEntity;
	};
	
	// Currently have to filter for closest entity, since QueryEntitiesBySphere is bugged
	// and somtimes queries entities outside the search distance
	protected bool ORS_EntitySearchCallback(IEntity entity)
	{
		float distanceSq = vector.DistanceSq(m_ORS_SearchPos, entity.GetOrigin());
		
		if (m_fORS_NearestDistanceSq > distanceSq)
		{
			m_fORS_NearestDistanceSq = distanceSq;
			m_eORS_NearestEntity = entity;
		};

		return (m_fORS_NearestDistanceSq != 0);
	};
};