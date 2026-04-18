import { motion } from 'framer-motion'
import { useAuth } from '../store/AuthContext'
import Button from '../components/Button'

const ETHNICITY_LABELS = {
  W_NL:  'White (Non-Latino)',
  HL:    'Hispanic / Latino',
  A:     'Asian',
  B_NL:  'Black (Non-Latino)',
  AI_AN: 'Am. Indian / AK Native',
  PI:    'Pacific Islander',
}

function Row({ label, value }) {
  if (!value && value !== 0) return null
  return (
    <div className="flex items-center justify-between py-3.5 border-b border-apple-gray-2 last:border-0">
      <span className="text-[15px] text-apple-sub">{label}</span>
      <span className="text-[15px] font-medium text-apple-text text-right max-w-[55%] truncate">{value}</span>
    </div>
  )
}

function BeliefBar({ score }) {
  return (
    <div className="mt-1">
      <div className="flex justify-between text-[11px] text-apple-sub mb-1.5">
        <span>Left</span>
        <span>Right</span>
      </div>
      <div className="h-2 bg-apple-gray-2 rounded-full overflow-hidden">
        <motion.div
          className="h-full rounded-full bg-gradient-to-r from-blue-500 to-apple-red"
          initial={{ width: 0 }}
          animate={{ width: `${score * 100}%` }}
          transition={{ duration: 0.8, ease: [0.25, 0.46, 0.45, 0.94] }}
        />
      </div>
    </div>
  )
}

function EnrichmentBadge({ status }) {
  const styles = {
    done:    'bg-apple-green/10 text-apple-green',
    pending: 'bg-apple-blue/10  text-apple-blue',
    failed:  'bg-apple-red/10   text-apple-red',
  }
  const labels = { done: 'Enriched', pending: 'Processing…', failed: 'Failed' }
  return (
    <span className={`text-[12px] font-semibold px-2.5 py-1 rounded-full ${styles[status] ?? styles.pending}`}>
      {labels[status] ?? 'Pending'}
    </span>
  )
}

export default function Profile() {
  const { user, logout } = useAuth()
  if (!user) return null

  return (
    <div className="flex-1 flex flex-col overflow-y-auto">
      {/* Header */}
      <div className="safe-top" />
      <div className="px-6 pt-10 pb-6 flex flex-col items-center text-center">
        <div className="w-24 h-24 rounded-full overflow-hidden bg-apple-gray mb-4 shadow-apple-sm">
          {user.picture ? (
            <img src={user.picture} alt={user.name} className="w-full h-full object-cover" />
          ) : (
            <div className="w-full h-full flex items-center justify-center">
              <span className="text-3xl font-bold text-apple-sub/40">{user.name?.[0]}</span>
            </div>
          )}
        </div>
        <h1 className="text-[24px] font-bold tracking-tight2 text-apple-text">{user.name}</h1>
        {user.location && (
          <p className="text-apple-sub text-[15px] mt-0.5 flex items-center gap-1">
            <svg width="12" height="12" viewBox="0 0 24 24" fill="currentColor">
              <path d="M12 2C8.13 2 5 5.13 5 9c0 5.25 7 13 7 13s7-7.75 7-13c0-3.87-3.13-7-7-7zm0 9.5a2.5 2.5 0 0 1 0-5 2.5 2.5 0 0 1 0 5z"/>
            </svg>
            {user.location}
          </p>
        )}
      </div>

      <div className="px-4 pb-10 flex flex-col gap-4">
        {/* Collected info */}
        <div className="bg-white rounded-3xl shadow-apple-sm px-5 py-1">
          <Row label="Age"    value={user.age} />
          <Row label="Email"  value={user.email} />
        </div>

        {/* Projected attributes */}
        <div>
          <div className="flex items-center justify-between px-2 mb-2">
            <p className="label">Projected</p>
            <EnrichmentBadge status={user.enrichment_status} />
          </div>
          <div className="bg-white rounded-3xl shadow-apple-sm px-5 py-1">
            <Row label="Gender"    value={user.projected_gender
              ? user.projected_gender.charAt(0).toUpperCase() + user.projected_gender.slice(1)
              : null}
            />
            <Row label="Ethnicity" value={ETHNICITY_LABELS[user.projected_ethnicity] ?? user.projected_ethnicity} />
            <Row label="Education" value={user.education_level?.replace(/_/g, ' ')} />
            {user.political_belief != null && (
              <div className="py-3.5">
                <span className="text-[15px] text-apple-sub">Political lean</span>
                <BeliefBar score={user.political_belief} />
              </div>
            )}
          </div>
        </div>

        {/* Sign out */}
        <Button variant="secondary" fullWidth onClick={logout}>
          Sign out
        </Button>
      </div>
    </div>
  )
}
