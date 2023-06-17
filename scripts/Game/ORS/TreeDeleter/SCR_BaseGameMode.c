/*
	Methods for deleting loadtime entities
*/

modded class SCR_BaseGameMode : BaseGameMode
{
	[RplProp(onRplName: "ORS_DeleteInitialEntityPositions")]
	protected ref array<vector> m_ORS_DeletedEntityPositions = {};
	protected IEntity m_eORS_NearestEntity;
	protected vector m_ORS_SearchPos;
	protected float m_fORS_NearestDistanceSq;
	protected const float ORS_SEARCH_DISTANCE = 0.01;
	
	// Ensures that already deleted loadtime entities are deleted for JIPs
	void ORS_DeleteInitialEntityPositions()
	{
		ORS_DeleteEntityPositionsLocal(m_ORS_DeletedEntityPositions);
	};
	
	// Deletes loadtime entities for all machines
	// Can only be called on the server
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void ORS_DeleteEntityPositionsGlobal(array<vector> entityPositions)
	{
		m_ORS_DeletedEntityPositions.InsertAll(entityPositions);
		ORS_DeleteEntityPositionsLocal(entityPositions);
		Rpc(ORS_DeleteEntityPositionsLocal, entityPositions);
	};

	// Deletes loadtime entities for local machine
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

	// Return all deleted loatime entities
	// Can only be called on the server
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
