class ORS_LoadtimeEntityDeleterComponentClass : ScriptComponentClass
{
};

class ORS_LoadtimeEntityDeleterComponent : ScriptComponent
{
	protected ref array<vector> m_DeletedEntityPositions = {};
	protected IEntity m_eNearestEntity;
	protected vector m_SearchPos;
	protected float m_fNearestDistanceSq;
	protected const float SEARCH_DISTANCE = 0.01;
		
	void DeleteEntityPositionsGlobal(array<vector> entityPositions)
	{
		m_DeletedEntityPositions.InsertAll(entityPositions);
		DeleteEntityPositionsLocal(entityPositions);
		Rpc(DeleteEntityPositionsLocal, entityPositions);
	};
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void DeleteEntityPositionsLocal(array<vector> entityPositions)
	{
		foreach (vector pos : entityPositions)
		{
			IEntity entity = GetNearestEntity(pos);
			if (entity)
				SCR_EntityHelper.DeleteEntityAndChildren(entity);
		};
	};
	
	array<vector> GetDeletedEntityPositions()
	{
		return m_DeletedEntityPositions;
	};
	
	protected IEntity GetNearestEntity(vector pos)
	{
		m_SearchPos = pos;
		m_fNearestDistanceSq = Math.Pow(SEARCH_DISTANCE, 2);
		m_eNearestEntity = null;
		GetGame().GetWorld().QueryEntitiesBySphere(m_SearchPos, SEARCH_DISTANCE, EntitySearchCallback);
		return m_eNearestEntity;
	};
	
	// Currently have to filter for closest entity, since QueryEntitiesBySphere is bugged
	// and somtimes queries entities outside the search distance
	protected bool EntitySearchCallback(IEntity entity)
	{
		float distanceSq = vector.DistanceSq(m_SearchPos, entity.GetOrigin());
		
		if (m_fNearestDistanceSq > distanceSq)
		{
			m_fNearestDistanceSq = distanceSq;
			m_eNearestEntity = entity;
		};

		return (m_fNearestDistanceSq != 0);
	}
};
